/**
 *  @file    SequentialTaskExecutorDaemon.h
 *  @author  Henri Casanova
 *  @date    2/22/2017
 *  @version 1.0
 *
 *  @brief WRENCH::SequentialTaskExecutorDaemon class implementation
 *
 *  @section DESCRIPTION
 *
 *  The WRENCH::SequentialTaskExecutorDaemon class implements the daemon for a simple
 *  Compute Service abstraction.
 *
 */

#ifndef SIMULATION_SEQUENTIALTASKEXECUTORDAEMON_H
#define SIMULATION_SEQUENTIALTASKEXECUTORDAEMON_H

#include "../../simgrid_util/DaemonWithMailbox.h"

namespace WRENCH {
		class SequentialTaskExecutorDaemon: public DaemonWithMailbox {

		public:
				SequentialTaskExecutorDaemon();
				~SequentialTaskExecutorDaemon();

		private:
				int main();

		};
}


#endif //SIMULATION_SEQUENTIALTASKEXECUTORDAEMON_H