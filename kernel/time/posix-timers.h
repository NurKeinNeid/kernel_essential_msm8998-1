#define TIMER_RETRY 1

struct k_clock {
	int (*clock_getres) (const clockid_t which_clock, struct timespec *tp);
	int (*clock_set) (const clockid_t which_clock,
			  const struct timespec *tp);
	int (*clock_get) (const clockid_t which_clock, struct timespec *tp);
	int (*clock_adj) (const clockid_t which_clock, struct timex *tx);
	int (*timer_create) (struct k_itimer *timer);
	int (*nsleep) (const clockid_t which_clock, int flags,
		       struct timespec *, struct timespec __user *);
	long (*nsleep_restart) (struct restart_block *restart_block);
	int (*timer_set) (struct k_itimer *timr, int flags,
			  struct itimerspec *new_setting,
			  struct itimerspec *old_setting);
	int (*timer_del) (struct k_itimer *timr);
	void (*timer_get) (struct k_itimer *timr,
			   struct itimerspec *cur_setting);
};

extern const struct k_clock clock_posix_cpu;
extern const struct k_clock clock_posix_dynamic;
extern const struct k_clock clock_process;
extern const struct k_clock clock_thread;
extern const struct k_clock alarm_clock;

int posix_timer_event(struct k_itimer *timr, int si_private);

void posix_cpu_timer_schedule(struct k_itimer *timer);

void common_timer_get(struct k_itimer *timr, struct itimerspec *cur_setting);
int common_timer_set(struct k_itimer *timr, int flags,
		     struct itimerspec *new_setting,
		     struct itimerspec *old_setting);
int common_timer_del(struct k_itimer *timer);
