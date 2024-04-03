#include "SSLSocket.h"

#include <iostream>

#ifdef WINDOWS_OS
#include <windows.h>
#else
#define DWORD unsigned long
#define u_long unsigned long

#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/socket.h>
#endif

#include "Logger.h"
#include "ThreadUtils.h"

int SSLSocket::nofSockets_ = 0;

void SSLSocket::create_context(const SSL_METHOD *method) {
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		throw std::runtime_error("Unable to create SSL context");
	}
}

void SSLSocket::handle_error(int result) {
	/*
	 * If something bad happened then we will dump the contents of the
	 * OpenSSL error stack to stderr. There might be some useful diagnostic
	 * information there.
	 */
	if (result == EXIT_FAILURE)
		ERR_print_errors_fp(stderr);

	/*
	 * Free the resources we allocated. We do not free the BIO object here
	 * because ownership of it was immediately transferred to the SSL object
	 * via SSL_set_bio(). The BIO will be freed when we free the SSL object.
	 */
	SSL_free(ssl);
	SSL_CTX_free(ctx);
}

void SSLSocket::configure_client_context() {
	/*
	 * Configure the client to abort the handshake if certificate
	 * verification fails. Virtually all clients should do this unless you
	 * really know what you are doing.
	 */
	bool verifyCertificates = true;
	if (std::getenv("SSL_VERIFY_CERT")) {
		verifyCertificates = strcmp(std::getenv("SSL_VERIFY_CERT"), "true") == 0;
	}
	SSL_CTX_set_verify(ctx, verifyCertificates ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, NULL);

	/* Use the default trusted certificate store */
	if (!SSL_CTX_set_default_verify_paths(ctx)) {
		handle_error(EXIT_FAILURE);
		throw std::runtime_error("Failed to set the default trusted certificate store\n");
	}

	/*
	 * In a real application you would probably just use the default system certificate trust store and call:
	 *     SSL_CTX_set_default_verify_paths(ctx);
	 * In this demo though we are using a self-signed certificate, so the client must trust it directly.
	 */
	//    if (!SSL_CTX_load_verify_locations(ctx, std::getenv("SSL_CERT_FILE_PATH"), NULL)) {
	//        ERR_print_errors_fp(stderr);
	//        exit(EXIT_FAILURE);
	//    }

	/*
	 * TLSv1.1 or earlier are deprecated by IETF and are generally to be
	 * avoided if possible. We require a minimum TLS version of TLSv1.2.
	 */
	if (!SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION)) {
		handle_error(EXIT_FAILURE);
		throw std::runtime_error("Failed to set the minimum TLS protocol version\n");
	}
}

void SSLSocket::configure_server_context() {
	if (!std::getenv("SSL_CERT_FILE_PATH")) {
		throw std::runtime_error("SSL_CERT_FILE_PATH environment variable is missing!");
	}

	if (!std::getenv("SSL_KEY_FILE_PATH")) {
		throw std::runtime_error("SSL_KEY_FILE_PATH environment variable is missing!");
	}

	std::string password;
	if (std::getenv("SSL_CERT_PASSPHRASE")) {
		password = std::getenv("SSL_CERT_PASSPHRASE");
	}

	// Set the userdata for the password callback.
	SSL_CTX_set_default_passwd_cb_userdata(ctx, &password);

	/* Set the key and cert */
	if (SSL_CTX_use_certificate_file(ctx, std::getenv("SSL_CERT_FILE_PATH"), SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, std::getenv("SSL_KEY_FILE_PATH"), SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
}

/**
 * SSLSocket - Constructor
 * Creates a SSLSocket and stores the socket file descriptor in s_
 */
SSLSocket::SSLSocket(const SSL_METHOD *method) : Socket(), ssl_method(method) { Init(); }

/**
 * SSLSocket - Constructor
 * Stores the socket file descriptor param in s_
 * @param s socket file descriptor
 */
SSLSocket::SSLSocket(SOCKET s, const SSL_METHOD *method) : Socket(s), ssl_method(method) { Init(); };

/**
 * SSLSocket Initialization
 */
void SSLSocket::Init() {
	create_context(ssl_method);

	if (ssl_method == TLS_server_method()) {
		configure_server_context();

		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, s_);

		if (SSL_accept(ssl) <= 0) {
			handle_error(EXIT_FAILURE);
			throw std::runtime_error("Failed to establish an SSL connection.");
		}
	} else if (ssl_method == TLS_client_method()) {
		configure_client_context();
	}
}

/**
 * Deconstructor
 */
SSLSocket::~SSLSocket() {
	if (!--(*refCounter_)) {
		Close();
		delete refCounter_;
	}

	--nofSockets_;
}

/**
 * SSLSocket - Constructor
 * @param o another socket object
 */
SSLSocket::SSLSocket(const SSLSocket &o) : Socket(o) {}

/**
 * SSLSocket - Constructor using the operator `=`
 * @param o another socket object
 */
SSLSocket &SSLSocket::operator=(const SSLSocket &o) {
	(*o.refCounter_)++;

	refCounter_ = o.refCounter_;
	s_ = o.s_;

	nofSockets_++;

	return *this;
}

/**
 * Method that receives the bytes from the socket file descriptor _s .
 * Assigns the bytes received to a string and then returns it.
 */
std::string SSLSocket::ReceiveBytes() {
	std::string ret;
	char buf[1024];

	while (true) {
		u_long arg = 1024;

		int rv = SSL_read(ssl, buf, arg);
		if (rv < 0) {
			int e = SSL_get_error(ssl, rv);
			switch (e) {
			case SSL_ERROR_ZERO_RETURN:
				/* End of data */
				Close();
				break;
			case SSL_ERROR_WANT_READ:
				LOG_ERROR("SSL_ERROR_WANT_READ");
				break;

				/* We get a WANT_WRITE if we're trying to rehandshake and we block on a write during that rehandshake.
				 * We need to wait on the socket to be writeable but reinitiate the read when it is
				 */
			case SSL_ERROR_WANT_WRITE:
				LOG_ERROR("SSL_ERROR_WANT_WRITE");
				break;
			default:
				break;
			}
			break;
		}

		std::string t;

		t.assign(buf, rv);
		ret += t;
	}
	return ret;
}

/**
 * Receives a line from the socket file descriptor
 */
std::string SSLSocket::ReceiveLine() {
	std::string ret;
	while (true) {
		char r;

		switch (SSL_read(ssl, &r, 1)) {
		case 0: // not connected anymore;
			// ... but last line sent
			// might not end in \n,
			// so return ret anyway.
			return ret;
		case -1:
			return "";
			//      if (errno == EAGAIN) {
			//        return ret;
			//      } else {
			//      // not connected anymore
			//      return "";
			//      }
		}

		ret += r;
		if (r == '\n')
			return ret;
	}
}

/**
 * Sends a line string to the socket file descriptor
 */
void SSLSocket::SendLine(std::string s) {
	s += '\n';
	SSL_write(ssl, s.c_str(), s.length());
}

/**
 * Sends string bytes to the socket file descriptor
 */
void SSLSocket::SendBytes(char *s, int length) {
	std::cout << "SendBytes -- " << "SSLSocket FD : " << s_ << std::endl;
	SSL_write(ssl, s, length);
}

void SSLSocket::Close() {
	SSL_shutdown(ssl);
	SSL_free(ssl);
	CloseSocket(s_);
}
