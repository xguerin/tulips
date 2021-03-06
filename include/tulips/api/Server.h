/*
 * Copyright (c) 2020, International Business Machines
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <tulips/api/Interface.h>
#ifdef TULIPS_ENABLE_ARP
#include <tulips/stack/arp/Processor.h>
#endif
#ifdef TULIPS_ENABLE_ICMP
#include <tulips/stack/icmpv4/Processor.h>
#endif
#include <tulips/stack/tcpv4/Processor.h>
#include <tulips/system/Compiler.h>
#include <tulips/transport/Device.h>
#include <map>
#include <unistd.h>

namespace tulips {

class Server
  : public interface::Server
  , public stack::tcpv4::EventHandler
{
public:
  Server(Delegate& delegate, transport::Device& device, const size_t nconn);

  inline Status run() override { return m_ethfrom.run(); }

  inline Status process(const uint16_t len, const uint8_t* const data) override
  {
    return m_ethfrom.process(len, data);
  }

  void listen(const stack::tcpv4::Port port, void* cookie) override;

  void unlisten(const stack::tcpv4::Port port) override;

  Status close(const ID id) override;

  bool isClosed(const ID id) const override;

  Status send(const ID id, const uint32_t len, const uint8_t* const data,
              uint32_t& off) override;

  /*
   * @param id the connection's handle.
   *
   * @return the user-allocate cookie for the connection.
   */
  void* cookie(const ID id) const;

private:
#ifdef TULIPS_ENABLE_RAW
  class RawProcessor : public Processor
  {
  public:
    Status run() override { return Status::Ok; }

    Status process(UNUSED const uint16_t len,
                   UNUSED const uint8_t* const data) override
    {
      return Status::Ok;
    }
  };
#endif

  void onConnected(stack::tcpv4::Connection& c) override;
  void onAborted(stack::tcpv4::Connection& c) override;
  void onTimedOut(stack::tcpv4::Connection& c) override;
  void onClosed(stack::tcpv4::Connection& c) override;

  Action onAcked(stack::tcpv4::Connection& c) override;

  Action onAcked(stack::tcpv4::Connection& c, const uint32_t alen,
                 uint8_t* const sdata, uint32_t& slen) override;

  Action onNewData(stack::tcpv4::Connection& c, const uint8_t* const data,
                   const uint32_t len) override;

  Action onNewData(stack::tcpv4::Connection& c, const uint8_t* const data,
                   const uint32_t len, const uint32_t alen,
                   uint8_t* const sdata, uint32_t& slen) override;

  void onSent(UNUSED stack::tcpv4::Connection& e) override {}

  Delegate& m_delegate;
  stack::ethernet::Producer m_ethto;
  stack::ipv4::Producer m_ip4to;
#ifdef TULIPS_ENABLE_ARP
  stack::arp::Processor m_arp;
#endif
  stack::ethernet::Processor m_ethfrom;
  stack::ipv4::Processor m_ip4from;
#ifdef TULIPS_ENABLE_ICMP
  stack::icmpv4::Processor m_icmpv4from;
#endif
#ifdef TULIPS_ENABLE_RAW
  RawProcessor m_raw;
#endif
  stack::tcpv4::Processor m_tcp;
  std::map<uint16_t, void*> m_cookies;
};

}
