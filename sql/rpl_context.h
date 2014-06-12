/* Copyright (c) 2014, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; version 2 of the
   License.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
   02110-1301 USA */

#ifndef RPL_SESSION_H
#define RPL_SESSION_H

#include "rpl_gtid.h"

/**
   This class is an interface for session consistency instrumentation
   in the server. It holds the context information for a given session.

   It does not require locking since access to this content is mutually
   exclusive by design (only one thread reading or writing to this object
   at a time).
 */
class Session_gtids_ctx
{
public:
  /**
   This is an interface to be implemented by classes that want to listen
   to changes to this context. This can be used, for instance, by the
   session tracker gtids to become aware of ctx modifications.
   */
  class Ctx_change_listener
  {
  public:
    Ctx_change_listener() {}
    virtual void notify_session_gtids_ctx_change()= 0;
  private:
    // not implemented
    Ctx_change_listener(const Ctx_change_listener& rsc);
    Ctx_change_listener& operator=(const Ctx_change_listener& rsc);
  };

private:

  /*
   Local sid_map to enable a lock free m_gtid_set.
   */
  Sid_map* m_sid_map;

  /**
    Set holding the transaction identifiers of the gtids
    to reply back on the response packet.

    Lifecycle: Emptied after the reply is sent back to the application. Remains
    empty until:
    - a RW transaction commits and a GTID is written to the binary log.
    - a RO transaction is issued, the consistency level is set to "Check
      Potential Writes" and the transaction is committed.
  */
  Gtid_set* m_gtid_set;

  /**
   If a listener is registered, e.g., the session track gtids, then this
   points to an instance of such listener.

   Since this context is valid only for one session, there is no need
   to protect this with locks.
  */
  Session_gtids_ctx::Ctx_change_listener* listener;

protected:

  /*
     Auxiliar function to determine if GTID collection should take place
     when it is invoked. It takes into consideration the gtid_mode and
     the current session context.

     @param thd the thread context.
     @return true if should collect gtids, false otherwise.
   */
  inline bool shall_collect(THD* thd);

  /**
   Auxiliar function that allows notification of ctx change listeners.
   */
  inline void notify_ctx_change_listener()
  {
    listener->notify_session_gtids_ctx_change();
  }

public:

  /**
    Simple constructor.
  */
  Session_gtids_ctx();

  /**
    The destructor. Deletes the m_gtid_set and the sid_map.
  */
  virtual ~Session_gtids_ctx();

  /**
   Unregisters the listener. The listener MUST have registered previously.

   @param listener a pointer to the listener to register.
  */
  void register_ctx_change_listener(Session_gtids_ctx::Ctx_change_listener* listener);

  /**
   Registers the listener. The pointer MUST not be NULL.

   @param listener a pointer to the listener to register.
   */
  void unregister_ctx_change_listener(Session_gtids_ctx::Ctx_change_listener* listener);

  /**
    This member function MUST return a reference to the set of collected
    GTIDs so far.

    @return the set of collected GTIDs so far.
   */
  inline Gtid_set* state() { return this->m_gtid_set; }

  /**
     This function MUST be called after the response packet is set to the
     client connected. The implementation may act on the collected state
     for instance to do garbage collection.

     @param thd The thread context.
   * @return true on error, false otherwise.
   */
  virtual bool notify_after_response_packet(THD* thd);

  /**
     This function MUST be called when relevant context is propagated throughout
     the replication protocol. This could mean, for instance, that it has
     been written to the binary log, thus slaves will get it.

     This function SHALL store the data if the thd->variables.session_track_gtids
     is set to a value other than NONE.

     @param thd   The thread context.
     @return true on error, false otherwise.
   */
  virtual bool notify_after_transaction_replicated(THD *thd);

  /**
     This function MUST be called after a transaction is committed
     in the server. It should be called regardless whether it is a
     RO or RW transaction. Also, DDLs, DDS are considered transaction
     for what is worth.

     This function SHALL store relevant data for the session consistency.

     @param thd    The thread context.
     @return true on error, false otherwise.
   */
  virtual bool notify_after_transaction_commit(THD* thd);

private:
  // not implemented
  Session_gtids_ctx(const Session_gtids_ctx& rsc);
  Session_gtids_ctx& operator=(const Session_gtids_ctx& rsc);
};

/*
  This class SHALL encapsulate the replication context associated with the THD
  object.
 */
class Rpl_thd_context
{
private:
  Session_gtids_ctx m_session_gtids_ctx;

  // make these private
  Rpl_thd_context(const Rpl_thd_context& rsc);
  Rpl_thd_context& operator=(const Rpl_thd_context& rsc);
public:

  Rpl_thd_context() { }

  inline Session_gtids_ctx& session_gtids_ctx()
  {
    return m_session_gtids_ctx;
  }
};

#endif	/* RPL_SESSION_H */