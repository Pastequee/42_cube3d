#include "../test.h"

void	__assert_str_rtn(const char *s1, const char *s2, const char *file, const char *function, int line)
{
	fprintf(stderr, "Assertion failed: (\"%s\" != \"%s\"), function %s, file %s, line %d.",
		s1, s2, function, file, line);
	exit(2);
}

void	print_pipe(int fd)
{
	int		ret;
	char	buf[512 + 1];

	do {
		ret = read(fd, buf, 512);
		buf[ret] = 0;
		printf("%s", buf);
	} while (ret != 0);
}

void	test_passed(char *name, double time)
{
	printf("\t%sPASS%s [% 6.2fs]\t\t%s%s%s\n", GREEN, RESET, time, BLUE, name, RESET);
}

void	print_test_fail_banner(t_test_infos *test)
{
	int		signal;
	char	*sig;

	sig = "";
	if (WIFSIGNALED(test->status)) {
		signal = WTERMSIG(test->status);
		if (signal == SIGSEGV)
			sig = "SIGSEGV";
		if (signal == SIGBUS)
			sig = "SIGBUS";
		if (signal == SIGPIPE)
			sig = "SIGPIPE";
		if (signal == SIGUSR1)
			sig = "TIMEOUT";
	}
	printf("\t%sFAIL%s [% 6.2fs]\t\t%s%s%s %s%s%s\n", RED, RESET, test->time, BLUE, test->name, RESET, RED, sig, RESET);
}

void	test_failed(t_test_infos *test, int stdout, int stderr)
{
	print_test_fail_banner(test);
	printf("\n%s--- STDOUT:%s           \t\t%s%s%s %s---%s\n", RED, RESET, BLUE, test->name, RESET, RED, RESET);
	print_pipe(stdout);
	printf("\n%s--- STDERR:%s           \t\t%s%s%s %s---%s\n", RED, RESET, BLUE, test->name, RESET, RED, RESET);
	print_pipe(stderr);
	printf("\n%s---%s\n", RED, RESET);
}

void	child_test(void (*f)(void), int fd_out[2], int fd_err[2])
{
	close(fd_out[0]);
	close(fd_err[0]);
	dup2(fd_out[1], STDOUT_FILENO);
	dup2(fd_err[1], STDERR_FILENO);
	f();
	exit(0);
}

double timediff(t_timeval end, t_timeval start)
{
	double	diff;

	diff = ((end.tv_sec - start.tv_sec) * 1000 + ((double)(end.tv_usec - start.tv_usec)) / 1000);
	return (diff / 1000);
}

int	settimeout(int pid, uint timeout)
{
	int	pid2;
	
	pid2 = fork();
	if (pid2 == -1) exit(4);
	if (pid2 == 0) {
		usleep(timeout * 1000);
		kill(pid, SIGUSR1);
		exit(0);
	}
	return (pid2);
}

bool	launch_test(t_test_infos *test, t_test_config *config)
{
	int				timer_pid;
	int				fd_out[2], fd_err[2];
	t_timeval 		start, end;
	
	if (pipe(fd_out) == -1) exit(1);
	if (pipe(fd_err) == -1) exit(2);

	gettimeofday(&start, NULL);
	test->pid = fork();
	if (test->pid == -1) exit(3);
	// Launch test function in a child process
	if (test->pid == 0)
		child_test(test->fun, fd_out, fd_err);

	// Main process here because child exit
	close(fd_out[1]);
	close(fd_err[1]);
	timer_pid = settimeout(test->pid, config->timeout);
	waitpid(test->pid, &test->status, 0);

	gettimeofday(&end, NULL);
	test->time = timediff(end, start);
	if (WIFEXITED(test->status) && WEXITSTATUS(test->status) == 0)
		test_passed(test->name, test->time);
	else {
		test_failed(test, fd_out[0], fd_err[0]);
		test->failed = true;
	}
	close(fd_out[0]);
	close(fd_err[0]);
	kill(timer_pid, SIGKILL);
	return (test->failed);
}

uint	nb_tests(t_test tests[])
{
	uint	i;

	i = 0;
	while (tests[i].f != NULL)
		i++;
	return (i);
}

void	fill_infos(t_test_infos	*infos, t_test *test)
{
	infos->failed = false;
	infos->time = 0;
	infos->pid = 0;
	infos->status = 0;
	infos->name = test->name;
	infos->fun = test->f;
}

void	run_tests(t_test tests[], t_test_config *config)
{
	uint			i, j, failed, nbtests;
	double			total_time;
	t_test_infos	*infos;

	i = failed = total_time = j = 0;
	nbtests = nb_tests(tests);
	infos = malloc(nbtests * sizeof(*infos));
	
	printf("    %sStarting%s %d tests\n", GREEN, RESET, nbtests);
	printf("------------\n");
	while (tests[i].f != NULL) {
		fill_infos(&infos[i], &tests[i]);
		launch_test(&infos[i], config);
		failed += infos[i].failed;
		if (config->stop_on_fail && infos[i].failed) {i++; break;}
		i++;
	}
	if (config->stop_on_fail && infos[i].failed)
		printf("%sCanceling due to test failure%s\n", RED, RESET);
	printf("------------\n");
	printf("     %sSummary%s [% 6.2fs] %d/%d tests run: %d %spassed%s, %d %sfailed%s\n",
		GREEN, RESET, total_time, i, nbtests, i - failed, GREEN, RESET, failed, RED, RESET);
	while (j < i) {
		if (infos[j].failed)
			print_test_fail_banner(&infos[j]);
		j++;
	}
	free(infos);
}

void	init_test_config(t_test_config *config)
{
	config->stop_on_fail = false;
	config->timeout = 1000;
}
