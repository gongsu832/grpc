/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef GRPCXX_CREDENTIALS_H
#define GRPCXX_CREDENTIALS_H

#include <memory>

#include <grpc++/impl/grpc_library.h>
#include <grpc++/support/config.h>

namespace grpc {
class ChannelArguments;
class Channel;
class SecureCredentials;

/// A credentials object encapsulates all the state needed by a client to
/// authenticate with a server and make various assertions, e.g., about the
/// client’s identity, role, or whether it is authorized to make a particular
/// call.
///
/// \see https://github.com/grpc/grpc/blob/master/doc/grpc-auth-support.md
class Credentials : public GrpcLibrary {
 public:
  ~Credentials() GRPC_OVERRIDE;

  /// Apply this instance's credentials to \a call.
  virtual bool ApplyToCall(grpc_call* call) = 0;

 protected:
  friend std::shared_ptr<Credentials> CompositeCredentials(
      const std::shared_ptr<Credentials>& creds1,
      const std::shared_ptr<Credentials>& creds2);

  virtual SecureCredentials* AsSecureCredentials() = 0;

 private:
  friend std::shared_ptr<Channel> CreateCustomChannel(
      const grpc::string& target, const std::shared_ptr<Credentials>& creds,
      const ChannelArguments& args);

  virtual std::shared_ptr<Channel> CreateChannel(
      const grpc::string& target, const ChannelArguments& args) = 0;
};

/// Options used to build SslCredentials.
struct SslCredentialsOptions {
  /// The buffer containing the PEM encoding of the server root certificates. If
  /// this parameter is empty, the default roots will be used.  The default
  /// roots can be overridden using the \a GRPC_DEFAULT_SSL_ROOTS_FILE_PATH
  /// environment variable pointing to a file on the file system containing the
  /// roots.
  grpc::string pem_root_certs;

  /// The buffer containing the PEM encoding of the client's private key. This
  /// parameter can be empty if the client does not have a private key.
  grpc::string pem_private_key;

  /// The buffer containing the PEM encoding of the client's certificate chain.
  /// This parameter can be empty if the client does not have a certificate
  /// chain.
  grpc::string pem_cert_chain;
};

// Factories for building different types of Credentials The functions may
// return empty shared_ptr when credentials cannot be created. If a
// Credentials pointer is returned, it can still be invalid when used to create
// a channel. A lame channel will be created then and all rpcs will fail on it.

/// Builds credentials with reasonable defaults.
///
/// \warning Only use these credentials when connecting to a Google endpoint.
/// Using these credentials to connect to any other service may result in this
/// service being able to impersonate your client for requests to Google
/// services.
std::shared_ptr<Credentials> GoogleDefaultCredentials();

/// Builds SSL Credentials given SSL specific options
std::shared_ptr<Credentials> SslCredentials(
    const SslCredentialsOptions& options);

/// Builds credentials for use when running in GCE
///
/// \warning Only use these credentials when connecting to a Google endpoint.
/// Using these credentials to connect to any other service may result in this
/// service being able to impersonate your client for requests to Google
/// services.
std::shared_ptr<Credentials> GoogleComputeEngineCredentials();

/// Builds Service Account JWT Access credentials.
/// json_key is the JSON key string containing the client's private key.
/// token_lifetime_seconds is the lifetime in seconds of each Json Web Token
/// (JWT) created with this credentials. It should not exceed
/// grpc_max_auth_token_lifetime or will be cropped to this value.
std::shared_ptr<Credentials> ServiceAccountJWTAccessCredentials(
    const grpc::string& json_key, long token_lifetime_seconds);

/// Builds refresh token credentials.
/// json_refresh_token is the JSON string containing the refresh token along
/// with a client_id and client_secret.
///
/// \warning Only use these credentials when connecting to a Google endpoint.
/// Using these credentials to connect to any other service may result in this
/// service being able to impersonate your client for requests to Google
/// services.
std::shared_ptr<Credentials> GoogleRefreshTokenCredentials(
    const grpc::string& json_refresh_token);

/// Builds access token credentials.
/// access_token is an oauth2 access token that was fetched using an out of band
/// mechanism.
///
/// \warning Only use these credentials when connecting to a Google endpoint.
/// Using these credentials to connect to any other service may result in this
/// service being able to impersonate your client for requests to Google
/// services.
std::shared_ptr<Credentials> AccessTokenCredentials(
    const grpc::string& access_token);

/// Builds IAM credentials.
///
/// \warning Only use these credentials when connecting to a Google endpoint.
/// Using these credentials to connect to any other service may result in this
/// service being able to impersonate your client for requests to Google
/// services.
std::shared_ptr<Credentials> GoogleIAMCredentials(
    const grpc::string& authorization_token,
    const grpc::string& authority_selector);

/// Combines two credentials objects into a composite credentials
std::shared_ptr<Credentials> CompositeCredentials(
    const std::shared_ptr<Credentials>& creds1,
    const std::shared_ptr<Credentials>& creds2);

/// Credentials for an unencrypted, unauthenticated channel
std::shared_ptr<Credentials> InsecureCredentials();

}  // namespace grpc

#endif  // GRPCXX_CREDENTIALS_H