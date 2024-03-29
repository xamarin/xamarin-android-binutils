// SPDX-License-Identifier: MIT
#include <cstring>
#include <cerrno>
#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include "constants.hh"
#include "process.hh"

using namespace xamarin::android::gas;

std::vector<platform::string::const_pointer> Process::make_exec_args ()
{
	std::vector<platform::string::const_pointer> exec_args;

	exec_args.push_back (
#if defined(_WIN32)
		_strdup (executable_path.string ().c_str ())
#else
		strdup (executable_path.string ().c_str ())
#endif
	);
	for (platform::string const& arg : _args) {
		exec_args.push_back (arg.c_str ());
	}

	return exec_args;
}

int Process::run (bool print_command_line)
{
	if (print_command_line) {
		print_process_command_line ();
	}

	std::vector<std::string::const_pointer> exec_args = make_exec_args ();

	ScopeGuard fg {
		[&]() -> void {
			// We made a copy in `make_exec_args`, free it up here
			free (const_cast<char*>(exec_args[0]));
		}
	};

	// `execv(2)` needs the array to be null-terminated
	exec_args.push_back (nullptr);

	pid_t llvm_mc_pid = fork ();
	if (llvm_mc_pid == -1) {
		STDERR << "Fork failed. " << std::strerror (errno) << Constants::newline;
		return Constants::wrapper_fork_failed_error_code;
	}

	if (llvm_mc_pid == 0) {
		if (execv (executable_path.c_str (), const_cast<char* const*>(exec_args.data ())) == -1) {
			STDERR << "Failed to run " << Constants::llvm_mc_name << ". " << std::strerror (errno) << Constants::newline;
		}
		_exit (Constants::wrapper_exec_failed_error_code);
	}

	int wstatus = 0;
	do {
		pid_t result = waitpid (llvm_mc_pid,  &wstatus, WUNTRACED);

		if (result == -1) {
			STDERR << "Failed to wait for " << Constants::llvm_mc_name << " to terminate. " << std::strerror (errno) << Constants::newline;
			return Constants::wrapper_wait_failed_error_code;
		}

		if (WIFSIGNALED (wstatus)) {
			STDERR << Constants::llvm_mc_name << " was killed by signal " << WTERMSIG (wstatus) << Constants::newline;
			return Constants::wrapper_llvm_mc_killed_error_code;
		} else if (WIFSTOPPED (wstatus)) {
			STDERR << Constants::llvm_mc_name << " was stopped by signal " << WSTOPSIG (wstatus) << Constants::newline;
			kill (llvm_mc_pid, SIGKILL); // Let's not risk hanging indifinitely...
			return Constants::wrapper_llvm_mc_stopped_error_code;
		}
	} while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));

	if (WEXITSTATUS (wstatus) != 0) {
		STDERR << Constants::llvm_mc_name << " exited with status " << WEXITSTATUS (wstatus) << Constants::newline;
	}
	return WEXITSTATUS (wstatus);
}
