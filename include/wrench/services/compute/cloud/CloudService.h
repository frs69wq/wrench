/**
 * Copyright (c) 2017-2018. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef WRENCH_CLOUDSERVICE_H
#define WRENCH_CLOUDSERVICE_H

#include "wrench/services/compute/virtualized_cluster/VirtualizedClusterService.h"
#include "CloudServiceProperty.h"
#include "CloudServiceMessagePayload.h"

namespace wrench {

    class Simulation;

    class ComputeService;

    /**
     * @brief A cloud-based compute service that manages a set of physical
     *        hosts and controls access to their resources by (transparently) executing jobs
     *        in VM instances.
     */
    class CloudService : public VirtualizedClusterService {

    private:
        std::map<std::string, std::string> default_property_values = {
                };

        std::map<std::string, std::string> default_messagepayload_values = {
                 {CloudServiceMessagePayload::STOP_DAEMON_MESSAGE_PAYLOAD,                  "1024"},
                 {CloudServiceMessagePayload::DAEMON_STOPPED_MESSAGE_PAYLOAD,               "1024"},
                 {CloudServiceMessagePayload::RESOURCE_DESCRIPTION_REQUEST_MESSAGE_PAYLOAD, "1024"},
                 {CloudServiceMessagePayload::RESOURCE_DESCRIPTION_ANSWER_MESSAGE_PAYLOAD,  "1024"},
                 {CloudServiceMessagePayload::GET_EXECUTION_HOSTS_REQUEST_MESSAGE_PAYLOAD,  "1024"},
                 {CloudServiceMessagePayload::GET_EXECUTION_HOSTS_ANSWER_MESSAGE_PAYLOAD,   "1024"},
                 {CloudServiceMessagePayload::CREATE_VM_REQUEST_MESSAGE_PAYLOAD,            "1024"},
                 {CloudServiceMessagePayload::CREATE_VM_ANSWER_MESSAGE_PAYLOAD,             "1024"},
                 {CloudServiceMessagePayload::SUBMIT_STANDARD_JOB_REQUEST_MESSAGE_PAYLOAD,  "1024"},
                 {CloudServiceMessagePayload::SUBMIT_STANDARD_JOB_ANSWER_MESSAGE_PAYLOAD,   "1024"},
                 {CloudServiceMessagePayload::SUBMIT_PILOT_JOB_REQUEST_MESSAGE_PAYLOAD,     "1024"},
                 {CloudServiceMessagePayload::SUBMIT_PILOT_JOB_ANSWER_MESSAGE_PAYLOAD,      "1024"}
                };

    public:
        CloudService(const std::string &hostname,
                     std::vector<std::string> &execution_hosts,
                     double scratch_space_size,
                     std::map<std::string, std::string> property_list = {},
                     std::map<std::string, std::string> messagepayload_list = {}
        );

    private:
        friend class Simulation;

        int main() override;
    };

}

#endif //WRENCH_CLOUDSERVICE_H
