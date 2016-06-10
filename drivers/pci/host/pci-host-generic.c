/*
 * Simple, generic PCI host controller driver targetting firmware-initialised
 * systems and virtual machines (e.g. the PCI emulation provided by kvmtool).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2014 ARM Limited
 *
 * Author: Will Deacon <will.deacon@arm.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_pci.h>
#include <linux/pci-ecam.h>
#include <linux/platform_device.h>

static struct pci_ecam_ops gen_pci_cfg_cam_bus_ops = {
	.bus_shift	= 16,
	.pci_ops	= {
		.map_bus	= pci_ecam_map_bus,
		.read		= pci_generic_config_read,
		.write		= pci_generic_config_write,
	}
};

static const struct of_device_id gen_pci_of_match[] = {
	{ .compatible = "pci-host-cam-generic",
	  .data = &gen_pci_cfg_cam_bus_ops },

	{ .compatible = "pci-host-ecam-generic",
	  .data = &pci_generic_ecam_ops },

	{ },
};

MODULE_DEVICE_TABLE(of, gen_pci_of_match);

static int pci_host_common_probe(struct platform_device *pdev,
				 struct gen_pci *pci)
{
	int err;
	const char *type;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct pci_bus *bus, *child;

	type = of_get_property(np, "device_type", NULL);
	if (!type || strcmp(type, "pci")) {
		dev_err(dev, "invalid \"device_type\" %s\n", type);
		return -EINVAL;
	}

	of_pci_check_probe_only();

	pci->host.dev.parent = dev;
	INIT_LIST_HEAD(&pci->host.windows);
	INIT_LIST_HEAD(&pci->resources);

	/* Parse our PCI ranges and request their resources */
	err = gen_pci_parse_request_of_pci_ranges(pci);
	if (err)
		return err;

	/* Parse and map our Configuration Space windows */
	err = gen_pci_parse_map_cfg_windows(pci);
	if (err) {
		gen_pci_release_of_pci_ranges(pci);
		return err;
	}

	/* Do not reassign resources if probe only */
	if (!pci_has_flag(PCI_PROBE_ONLY))
		pci_add_flags(PCI_REASSIGN_ALL_RSRC | PCI_REASSIGN_ALL_BUS);


	bus = pci_scan_root_bus(dev, pci->cfg.bus_range->start,
				&pci->cfg.ops->ops, pci, &pci->resources);
	if (!bus) {
		dev_err(dev, "Scanning rootbus failed");
		return -ENODEV;
	}

	pci_fixup_irqs(pci_common_swizzle, of_irq_parse_and_map_pci);

	if (!pci_has_flag(PCI_PROBE_ONLY)) {
		pci_bus_size_bridges(bus);
		pci_bus_assign_resources(bus);

		list_for_each_entry(child, &bus->children, node)
			pcie_bus_configure_settings(child);
	}

	pci_bus_add_devices(bus);
	return 0;
}

static int gen_pci_probe(struct platform_device *pdev)
{
	const struct of_device_id *of_id;
 	of_id = of_match_node(gen_pci_of_match, pdev->dev.of_node);
	pci->cfg.ops = (struct pci_ecam_ops *)of_id->data;
 	return pci_host_common_probe(pdev, ops);
}

static struct platform_driver gen_pci_driver = {
	.driver = {
		.name = "pci-host-generic",
		.of_match_table = gen_pci_of_match,
	},
	.probe = gen_pci_probe,
};
module_platform_driver(gen_pci_driver);

MODULE_DESCRIPTION("Generic PCI host driver");
MODULE_AUTHOR("Will Deacon <will.deacon@arm.com>");
MODULE_LICENSE("GPL v2");
