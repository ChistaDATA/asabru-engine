//
// Created by Midhun Darvin on 04/06/24.
//

#include "CApiGatewaySocket.h"

bool CApiGatewaySocket::Start(std::string identifier) {
	return Open(std::move(identifier), thread_routine_override); // Call the base class's Open function
}


