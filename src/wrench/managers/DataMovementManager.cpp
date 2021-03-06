/**
 * Copyright (c) 2017. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <wrench/simgrid_S4U_util/S4U_Simulation.h>
#include <wrench/logging/TerminalOutput.h>
#include <wrench/simgrid_S4U_util/S4U_Mailbox.h>
#include <wrench/simulation/SimulationMessage.h>
#include <wrench/services/ServiceMessage.h>
#include <wrench/services/storage/StorageService.h>
#include <wrench/services/file_registry/FileRegistryService.h>
#include <wrench/exceptions/WorkflowExecutionException.h>
#include <services/storage/StorageServiceMessage.h>
#include <wrench/workflow/WorkflowFile.h>
#include <wrench/wms/WMS.h>
#include "wrench/workflow/Workflow.h"
#include "wrench/managers/DataMovementManager.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(data_movement_manager, "Log category for Data Movement Manager");

namespace wrench {



    /**
     * @brief Constructor
     *
     * @param wms: the WMS that uses this data movement manager
     */
    DataMovementManager::DataMovementManager(WMS *wms) :
            Service(wms->hostname, "data_movement_manager", "data_movement_manager") {

      this->wms = wms;

      // Get myself known to schedulers
      if (this->wms->standard_job_scheduler) {
        this->wms->standard_job_scheduler->setDataMovementManager(this);
      }
      if (this->wms->pilot_job_scheduler) {
        this->wms->pilot_job_scheduler->setDataMovementManager(this);
      }

    }

    /**
     * @brief Kill the manager (brutally terminate the daemon)
     */
    void DataMovementManager::kill() {
      this->killActor();
    }

    /**
     * @brief Stop the manager
     *
     * @throw WorkflowExecutionException
     * @throw std::runtime_error
     */
    void DataMovementManager::stop() {
      try {
        S4U_Mailbox::putMessage(this->mailbox_name, new ServiceStopDaemonMessage("", 0.0));
      } catch (std::shared_ptr<NetworkError> &cause) {
        throw WorkflowExecutionException(cause);
      }
    }

    /**
     * @brief Ask the data manager to initiate an asynchronous file copy
     * @param file: the file to copy
     * @param src: the source storage service (using the "/" partition)
     * @param dst: the destination storage service (using the "/" partition)
     * @param file_registry_service: a file registry service to update once the file copy has (successfully) completed (none if nullptr)
     *
     * @throw std::invalid_argument
     * @throw WorkflowExecutionException
     */
    void DataMovementManager::initiateAsynchronousFileCopy(WorkflowFile *file,
                                                           StorageService *src,
                                                           StorageService *dst,
                                                           FileRegistryService *file_registry_service) {
      initiateAsynchronousFileCopy(file, src, "/", dst, "/", file_registry_service);
    }

    /**
     * @brief Ask the data manager to initiate an asynchronous file copy
     * @param file: the file to copy
     * @param src: the source storage service
     * @param src_partition: the source partition
     * @param dst: the destination storage service
     * @param dst_partition: the destination partition
     * @param file_registry_service: a file registry service to update once the file copy has (successfully) completed (none if nullptr)
     *
     * @throw std::invalid_argument
     * @throw WorkflowExecutionException
     */
    void DataMovementManager::initiateAsynchronousFileCopy(WorkflowFile *file,
                                                           StorageService *src,
                                                           std::string src_partition,
                                                           StorageService *dst,
                                                           std::string dst_partition,
                                                           FileRegistryService *file_registry_service) {
      if ((file == nullptr) || (src == nullptr) || (dst == nullptr)) {
        throw std::invalid_argument("DataMovementManager::initiateFileCopy(): Invalid arguments");
      }
      if (src_partition.empty()) {
        src_partition = "/";
      }
      if (dst_partition.empty()) {
        dst_partition = "/";
      }

      DataMovementManager::CopyRequestSpecs request(file, dst, dst_partition, file_registry_service);

      try {
        for (auto const &p : this->pending_file_copies) {
          if (*p == request) {
            throw WorkflowExecutionException(std::shared_ptr<FailureCause>(new FileAlreadyBeingCopied(file, dst, dst_partition)));
          }
        }
      } catch (WorkflowExecutionException &e) {
        throw;
      }


      try {
        this->pending_file_copies.push_front(std::unique_ptr<CopyRequestSpecs>(new CopyRequestSpecs(file, dst, dst_partition, file_registry_service)));
        dst->initiateFileCopy(this->mailbox_name, file, src, src_partition, dst_partition);
      } catch (WorkflowExecutionException &e) {
        throw;
      }
    }

    /**
     * @brief Ask the data manager to perform a synchronous file copy
     * @param file: the file to copy
     * @param src: the source storage service (using the "/" partition)
     * @param dst: the destination storage service (using the "/" partition)
     * @param file_registry_service: a file registry service to update once the file copy has (successfully) completed (none if nullptr)
     *
     * @throw std::invalid_argument
     * @throw WorkflowExecutionException
     */
    void DataMovementManager::doSynchronousFileCopy(WorkflowFile *file,
                                                    StorageService *src,
                                                    StorageService *dst,
                                                    FileRegistryService *file_registry_service) {
      doSynchronousFileCopy(file, src, "/", dst, "/", file_registry_service);
    }

    /**
     * @brief Ask the data manager to perform a synchronous file copy
     * @param file: the file to copy
     * @param src: the source storage service
     * @param dst: the destination storage service
     * @param file_registry_service: a file registry service to update once the file copy has (successfully) completed (none if nullptr)
     *
     * @throw std::invalid_argument
     * @throw WorkflowExecutionException
     */
    void DataMovementManager::doSynchronousFileCopy(WorkflowFile *file,
                                                    StorageService *src,
                                                    std::string src_partition,
                                                    StorageService *dst,
                                                    std::string dst_partition,
                                                    FileRegistryService *file_registry_service) {
      if ((file == nullptr) || (src == nullptr) || (dst == nullptr)) {
        throw std::invalid_argument("DataMovementManager::initiateFileCopy(): Invalid arguments");
      }
      if (src_partition.empty()) {
        src_partition = "/";
      }
      if (dst_partition.empty()) {
        dst_partition = "/";
      }

      DataMovementManager::CopyRequestSpecs request(file, dst, dst_partition, file_registry_service);

      try {
        for (auto const &p : this->pending_file_copies) {
          if (*p == request) {
            throw WorkflowExecutionException(std::shared_ptr<FailureCause>(new FileAlreadyBeingCopied(file, dst, dst_partition)));
          }
        }

        dst->copyFile(file, src);
      } catch (WorkflowExecutionException &e) {
        throw;
      }

      try {
        if (file_registry_service) {
          file_registry_service->addEntry(file, dst);
        }
      } catch (WorkflowExecutionException &e) {
        throw;
      }
    }



/**
 * @brief Main method of the daemon that implements the DataMovementManager
 * @return 0 on success
 */
    int DataMovementManager::main() {

      TerminalOutput::setThisProcessLoggingColor(TerminalOutput::COLOR_YELLOW);

      WRENCH_INFO("New Data Movement Manager starting (%s)", this->mailbox_name.c_str());

      while (processNextMessage()) {



      }

      WRENCH_INFO("Data Movement Manager terminating");

      return 0;
    }

/**
 * @brief Process the next message
 * @return true if the daemon should continue, false otherwise
 *
 * @throw std::runtime_error
 */
    bool DataMovementManager::processNextMessage() {

      std::unique_ptr<SimulationMessage> message = nullptr;

      try {
        message = S4U_Mailbox::getMessage(this->mailbox_name);
      } catch (std::shared_ptr<NetworkError> &cause) {
        return true;
      }

      if (message == nullptr) {
        WRENCH_INFO("Got a NULL message... Likely this means we're all done. Aborting!");
        return false;
      }

      WRENCH_INFO("Data Movement Manager got a %s message", message->getName().c_str());

      if (auto msg = dynamic_cast<ServiceStopDaemonMessage *>(message.get())) {
        // There shouldn't be any need to clean any state up
        return false;

      } else if (auto msg = dynamic_cast<StorageServiceFileCopyAnswerMessage *>(message.get())) {

        // Remove the record and find the File Registry Service, if any
        DataMovementManager::CopyRequestSpecs request(msg->file, msg->storage_service, msg->dst_partition, nullptr);
        for (auto it = this->pending_file_copies.begin();
             it != this->pending_file_copies.end();
             ++it) {
          if (*(*it) == request) {
            request.file_registry_service = (*it)->file_registry_service;
            this->pending_file_copies.erase(it); // remove the entry
            break;
          }
        }

        bool file_registry_service_updated = false;
        if (request.file_registry_service) {
          WRENCH_INFO("Trying to do a register");
          try {
            request.file_registry_service->addEntry(request.file, request.dst);
            file_registry_service_updated = true;
          } catch (WorkflowExecutionException &e) {
            WRENCH_INFO("Oops, couldn't do it");
            // don't throw, just keep file_registry_service_update to false
          }
        }

          WRENCH_INFO("Forwarding status message");
        // Forward it back
        try {
          S4U_Mailbox::dputMessage(msg->file->getWorkflow()->getCallbackMailbox(),
                                   new StorageServiceFileCopyAnswerMessage(request.file,
                                                                           request.dst,
                                                                           request.dst_partition,
                                                                           request.file_registry_service,
                                                                           file_registry_service_updated,
                                                                           msg->success,
                                                                           std::move(msg->failure_cause), 0));
        } catch  (std::shared_ptr<NetworkError> &cause) {
          return true;
        }
        return true;

      } else {
        throw std::runtime_error("DataMovementManager::waitForNextMessage(): Unexpected [" + message->getName() + "] message");
      }

    }


};