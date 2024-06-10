/***********************************************************************************************
 * File: ProxySocket.h
 * Description: This header file defines the CProxySocket class, which is derived from CServerSocket.
 *              CProxySocket is used for managing proxy server functionality.
 ***********************************************************************************************/

#pragma once

#include "../socket/CServerSocket.h"           // Include the base class header
#include "CProxyHandler.h"       // Include the proxy handler header
#include "config/config_types.h"        // Include the configuration types header
#include "CommonTypes.h"
#include "ProtocolHelper.h"
#include "pipeline/Pipeline.h"

// Class definition for CProxySocket
class CProxySocket : public CServerSocket {
    CProxyHandler *m_handler = nullptr; // Pointer to the proxy handler instance
    TARGET_ENDPOINT_CONFIG m_configValues; // Configuration values for the proxy socket

    // Static thread handler function for internal use
    static void *ThreadHandler(CProxySocket *ptr, void *lptr);

    // Function pointer for the thread routine
    std::function<void *(void *)> thread_routine_override = nullptr;

public:
    LoadBalancerStrategy<RESOLVED_SERVICE> *loadBalancerStrategy;
    LoadBalancer<RESOLVED_SERVICE> *loadBalancer;
	std::map<std::string, RESOLVED_SERVICE> *hostname_map;

    // Constructor: Initializes the CProxySocket instance
    explicit CProxySocket(int port, LoadBalancerStrategy<RESOLVED_SERVICE> *loadBalancerStrategy = new RoundRobinStrategy<RESOLVED_SERVICE>)
        : CServerSocket(port), loadBalancerStrategy(loadBalancerStrategy) {
        // Create a lambda function that wraps the ThreadHandler
        std::function<void *(void *)> pipelineLambda = [this](void *ptr) -> void * {
            return CProxySocket::ThreadHandler(this, ptr);
        };
        thread_routine_override = pipelineLambda; // Set the thread routine
    }

    // Set the proxy handler for the socket
    bool SetHandler(CProxyHandler *ph) {
        m_handler = ph;
        return m_handler != nullptr;
    }

    // Set a custom pipeline function for the socket
    bool SetPipeline(PipelineFunction<CProxySocket> pipelineFunction) {
        if (pipelineFunction == nullptr) {
            return false;
        }
        // Create a lambda that wraps the provided pipeline function
        std::function<void *(void *)> pipelineLambda = [this, pipelineFunction](void *ptr) -> void * {
            return pipelineFunction(this, ptr);
        };
        thread_routine_override = pipelineLambda; // Set the thread routine to the pipeline

        return true;
    }

    // Get the proxy handler instance
    CProxyHandler *GetHandler() { return m_handler; }

    // Set configuration values for the proxy socket
    bool SetConfigValues(TARGET_ENDPOINT_CONFIG configValues) {
        m_configValues = configValues;
        return true;
    }

    // Get configuration values for the proxy socket
    TARGET_ENDPOINT_CONFIG GetConfigValues() {
        return m_configValues;
    }

    // Start the proxy socket
    bool Start(std::string identifier) {
        /**
         * Initialize Load balancer for the proxy socket
         */
        loadBalancer = new LoadBalancer(this->loadBalancerStrategy);

		/**
         * Get the configuration data for the target database clusters ( eg. clickhouse )
         * Config given in config.xml
		 */
		TARGET_ENDPOINT_CONFIG targetEndpointConfig = this->GetConfigValues();

		this->hostname_map = new std::map<std::string, RESOLVED_SERVICE>();

		for (const auto& service: targetEndpointConfig.services) {
			// add service to load balancer
			loadBalancer->addServer(service);

			// Set hostname config map if available
			if (!service.source_hostname.empty())
				(*hostname_map)[service.source_hostname] = service;
		}

        return Open(std::move(identifier), thread_routine_override); // Call the base class's Open function
    }
};
