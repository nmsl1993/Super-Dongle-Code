/**
  ******************************************************************************
  * @file    udp_echoserver.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   UDP echo server
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "pb_decode.h"
  #include "pb_encode.h"

#include "message.pb.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void udp_echoserver_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
struct ip_addr DestIPaddr;

/* Private functions ---------------------------------------------------------*/
   struct udp_pcb *upcb;
struct pbuf *proto_out;

DSPResponse dsp_resp;
#define PBUF_OUT_PAYLOAD_SIZE 32
const pb_field_t* decode_command_type(pb_istream_t *stream)
{
    pb_wire_type_t wire_type;
    uint32_t tag;
    bool eof;

    while (pb_decode_tag(stream, &wire_type, &tag, &eof))
    {
        if (wire_type == PB_WT_STRING)
        {
            const pb_field_t *field;
            for (field = Command_fields; field->tag != 0; field++)
            {
                if (field->tag == tag && (field->type & PB_LTYPE_SUBMESSAGE))
                {
                    /* Found our field. */
                    return field->ptr;
                }
            }
        }
        
        /* Wasn't our field.. */
        pb_skip_field(stream, wire_type);
    }
    
    return NULL;
}

bool decode_command_contents(pb_istream_t *stream, const pb_field_t fields[], void *dest_struct)
{
    pb_istream_t substream;
    bool status;
    if (!pb_make_string_substream(stream, &substream))
        return false;
    
    status = pb_decode(&substream, fields, dest_struct);
    pb_close_string_substream(stream, &substream);
    return status;
}
/**
  * @brief  Initialize the server application.
  * @param  None
  * @retval None
  */
void udp_echoserver_init(void)
{
   err_t err;
    IP4_ADDR(&DestIPaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3 );

   /* Create a new UDP control block  */
   upcb = udp_new();
  proto_out = pbuf_alloc(PBUF_RAM, PBUF_OUT_PAYLOAD_SIZE, PBUF_RAM);

   if (upcb)
   {
     /* Bind the upcb to the UDP_PORT port */
     /* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
      err = udp_bind(upcb, IP_ADDR_ANY, UDP_SERVER_COMMAND_PORT);
      
      if(err == ERR_OK)
      {
        /* Set a receive callback for the upcb */
        udp_recv(upcb, udp_echoserver_receive_callback, NULL);
      }
      else
      {
        udp_remove(upcb);
        //printf("can not bind pcb");
      }
   }
   else
   {
     //printf("can not create pcb");
   } 
}
void sendFreqs(float * freqs)
{
  pb_ostream_t stream = pb_ostream_from_buffer(proto_out->payload,PBUF_OUT_PAYLOAD_SIZE);

  dsp_resp.freq1 = freqs[0];
  dsp_resp.freq2 = freqs[1];
  dsp_resp.freq3 = freqs[2];

  pb_encode(&stream,DSPResponse_fields,&dsp_resp);
  int msg_len = stream.bytes_written;

  udp_sendto(upcb, proto_out, &DestIPaddr, UDP_CLIENT_COMMAND_PORT );

}
/**
  * @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param pcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  * @retval None
  */
void udp_echoserver_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
  //pb_istream_t stream = pb_istream_from_buffer(p->payload,p->len;
    pb_istream_t stream = pb_istream_from_buffer(p->payload, p->len);
    
    const pb_field_t *type = decode_command_type(&stream);
    bool status = false;
    
    if (type == PGAGainCommand_fields)
    {

        PGAGainCommand msg = {};
        status = decode_command_contents(&stream, PGAGainCommand_fields, &msg);
        setGain(msg.gain);
    }
    else if (type == LEDCommand_fields)
    {
        LEDCommand msg = {};
        status = decode_command_contents(&stream, LEDCommand_fields, &msg);
        if(msg.on)
        {
        GPIO_ResetBits(GPIOE,LED3);
        }
        else
        {
        GPIO_SetBits(GPIOE,LED3);
        }
    }
    
    if (!status)
    {
        //printf("Decode failed: %s\n", PB_GET_ERROR(&stream));
    }
    
  /* Connect to the remote client */
  udp_connect(upcb, addr, UDP_CLIENT_COMMAND_PORT);
    
  /* Tell the client that we have accepted it */
  udp_send(upcb, p);

  /* free the UDP connection, so we can accept new clients */
  udp_disconnect(upcb);
	
  /* Free the p buffer */
  pbuf_free(p);
   
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
