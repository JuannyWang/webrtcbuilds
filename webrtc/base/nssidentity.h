/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_BASE_NSSIDENTITY_H_
#define WEBRTC_BASE_NSSIDENTITY_H_

#include <string>

// Hack: Define+undefine int64 and uint64 to avoid typedef conflict with NSS.
// TODO(kjellander): Remove when webrtc:4497 is completed.
#define uint64 foo_uint64
#define int64 foo_int64
#include "cert.h"
#undef uint64
#undef int64
#include "nspr.h"
#include "hasht.h"
#include "keythi.h"

#include "webrtc/base/common.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/base/sslidentity.h"

namespace rtc {

class NSSKeyPair {
 public:
  NSSKeyPair(SECKEYPrivateKey* privkey, SECKEYPublicKey* pubkey) :
      privkey_(privkey), pubkey_(pubkey) {}
  ~NSSKeyPair();

  // Generate a 1024-bit RSA key pair.
  static NSSKeyPair* Generate();
  NSSKeyPair* GetReference();

  SECKEYPrivateKey* privkey() const { return privkey_; }
  SECKEYPublicKey * pubkey() const { return pubkey_; }

 private:
  SECKEYPrivateKey* privkey_;
  SECKEYPublicKey* pubkey_;

  DISALLOW_COPY_AND_ASSIGN(NSSKeyPair);
};


class NSSCertificate : public SSLCertificate {
 public:
  static NSSCertificate* FromPEMString(const std::string& pem_string);
  // The caller retains ownership of the argument to all the constructors,
  // and the constructor makes a copy.
  explicit NSSCertificate(CERTCertificate* cert);
  explicit NSSCertificate(CERTCertList* cert_list);
  ~NSSCertificate() override;

  NSSCertificate* GetReference() const override;

  std::string ToPEMString() const override;

  void ToDER(Buffer* der_buffer) const override;

  bool GetSignatureDigestAlgorithm(std::string* algorithm) const override;

  bool ComputeDigest(const std::string& algorithm,
                     unsigned char* digest,
                     size_t size,
                     size_t* length) const override;

  bool GetChain(SSLCertChain** chain) const override;

  CERTCertificate* certificate() { return certificate_; }

  // Performs minimal checks to determine if the list is a valid chain.  This
  // only checks that each certificate certifies the preceding certificate,
  // and ignores many other certificate features such as expiration dates.
  static bool IsValidChain(const CERTCertList* cert_list);

  // Helper function to get the length of a digest
  static bool GetDigestLength(const std::string& algorithm, size_t* length);

  // Comparison.  Only the certificate itself is considered, not the chain.
  bool Equals(const NSSCertificate* tocompare) const;

 private:
  NSSCertificate(CERTCertificate* cert, SSLCertChain* chain);
  static bool GetDigestObject(const std::string& algorithm,
                              const SECHashObject** hash_object);

  CERTCertificate* certificate_;
  scoped_ptr<SSLCertChain> chain_;

  DISALLOW_COPY_AND_ASSIGN(NSSCertificate);
};

// Represents a SSL key pair and certificate for NSS.
class NSSIdentity : public SSLIdentity {
 public:
  static NSSIdentity* Generate(const std::string& common_name);
  static NSSIdentity* GenerateForTest(const SSLIdentityParams& params);
  static SSLIdentity* FromPEMStrings(const std::string& private_key,
                                     const std::string& certificate);
  ~NSSIdentity() override;

  NSSIdentity* GetReference() const override;
  NSSCertificate& certificate() const override;

  NSSKeyPair* keypair() const { return keypair_.get(); }

 private:
  NSSIdentity(NSSKeyPair* keypair, NSSCertificate* cert);

  static NSSIdentity* GenerateInternal(const SSLIdentityParams& params);

  rtc::scoped_ptr<NSSKeyPair> keypair_;
  rtc::scoped_ptr<NSSCertificate> certificate_;

  DISALLOW_COPY_AND_ASSIGN(NSSIdentity);
};

}  // namespace rtc

#endif  // WEBRTC_BASE_NSSIDENTITY_H_
