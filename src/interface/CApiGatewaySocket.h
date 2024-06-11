#pragma once
#include "CProxySocket.h"

class CApiGatewaySocket : public CProxySocket {
  public:
	std::map<std::string, std::map<std::string, RESOLVED_SERVICE>> hostname_map;
	bool Start(std::string identifier) override;

	explicit CApiGatewaySocket(int port): CProxySocket(port) {}

	void SetApiGatewayConfig(const std::map<std::string, std::map<std::string, RESOLVED_SERVICE>>& h_map) {
		hostname_map = h_map;
	}

	// Set a custom pipeline function for the socket
	bool SetPipeline(PipelineFunction<CApiGatewaySocket> pipelineFunction) {
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
};

