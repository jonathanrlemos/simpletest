/** @file tests/cstest/cstest_signal.cpp
 * @brief CloudSync testing framework signal handler.
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "simpletest_signal.hpp"
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>

namespace simpletest{

thread_local int instanceCount = 0;
thread_local jmp_buf s_jmpbuf;
thread_local volatile sig_atomic_t s_signo = 0;
thread_local volatile bool s_exit = false;

static const sig_atomic_t signals[] = {SIGINT, SIGABRT, SIGSEGV, SIGTERM};

static void handler(int signo){
	switch (signo){
	case SIGABRT:
	case SIGINT:
	case SIGTERM:
		s_exit = true;
	}
	s_signo = signo;
	longjmp(s_jmpbuf, signo);
}

SignalHandler::SignalHandler(){
	struct sigaction sa;

	if (instanceCount > 0){
		throw new std::logic_error("Cannot have more than one instance of signal handler at a time in one thread.");
	}
	instanceCount++;

	sa.sa_handler = handler;
	sigfillset(&(sa.sa_mask));
	sa.sa_flags = SA_RESTART;

	for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); ++i){
		sigaction(signals[i], &sa, NULL);
	}

	if (setjmp(s_jmpbuf)){
		std::cerr << "An ActivateSignalHandler() location was not specified." << std::endl;
		std::exit(1);
	}
}

SignalHandler::~SignalHandler(){
	struct sigaction sa;

	instanceCount--;

	sa.sa_handler = SIG_DFL;
	sigfillset(&(sa.sa_mask));
	sa.sa_flags = SA_RESTART;

	for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); ++i){
		sigaction(signals[i], &sa, NULL);
	}
}

sig_atomic_t SignalHandler::lastSignal(){
	return s_signo;
}

const char* SignalHandler::signalToString(sig_atomic_t signo){
	switch (signo){
	case SIGINT:
		return "Interrupt signal";
	case SIGABRT:
		return "Abort signal";
	case SIGSEGV:
		return "Segmentation fault";
	case SIGTERM:
		return "Termination signal";
	default:
		return "Unknown signal";
	}
}

jmp_buf& SignalHandler::getBuf(){
	return s_jmpbuf;
}

bool SignalHandler::shouldExit(){
	return s_exit;
}

}
