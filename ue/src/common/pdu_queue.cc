/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2015 Software Radio Systems Limited
 *
 * \section LICENSE
 *
 * This file is part of the srsUE library.
 *
 * srsUE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsUE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */


#define Error(fmt, ...)   log_h->error_line(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Warning(fmt, ...) log_h->warning_line(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Info(fmt, ...)    log_h->info_line(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Debug(fmt, ...)   log_h->debug_line(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#include "common/pdu_queue.h"


namespace srslte {
    

void pdu_queue::init(process_callback *callback_, log* log_h_)
{
  callback  = callback_;
  log_h     = log_h_;   
}

uint8_t* pdu_queue::request(uint32_t len)
{  
  if (len > MAX_PDU_LEN) {
    fprintf(stderr, "Error request buffer of invalid size %d. Max bytes %d\n", len, MAX_PDU_LEN);
    return NULL; 
  }
  
  pdu_t *pdu = pool.allocate();  
  if (!pdu) {
    if (log_h) {
      log_h->error("Not enough buffers for MAC PDU\n");      
    }
    fprintf(stderr, "Not enough buffers for MAC PDU\n");
  }
  if ((void*) pdu->ptr != (void*) pdu) {
    fprintf(stderr, "Fatal error in memory alignment in struct pdu_queue::pdu_t\n");
    exit(-1);
  }
  
  return pdu->ptr; 
}

/* Demultiplexing of logical channels and dissassemble of MAC CE 
 * This function enqueues the packet and returns quicly because ACK 
 * deadline is important here. 
 */ 
void pdu_queue::push(uint8_t *ptr, uint32_t len)
{
  pdu_t *pdu = (pdu_t*) ptr; 
  pdu->len   = len; 
  pdu_q.push(pdu);    
}

bool pdu_queue::process_pdus()
{
  bool have_data = false; 
  uint32_t cnt  = 0; 
  pdu_t *pdu; 
  while(pdu_q.try_pop(&pdu)) {
    if (callback) {
      callback->process_pdu(pdu->ptr, pdu->len);
    }
    pool.deallocate(pdu);
    cnt++;
    have_data = true;
  }
  if (cnt > 20) {
    if (log_h) {
      log_h->warning("Warning PDU queue dispatched %d packets\n", cnt);
    }
    printf("Warning PDU queue dispatched %d packets\n", cnt);
  }
  return have_data; 
}

}
