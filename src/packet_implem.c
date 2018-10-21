#include "packet_interface.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <zlib.h>
#include <sys/types.h>



struct __attribute__((__packed__)) pkt {
uint8_t window : 5;
uint8_t tr : 1;
uint8_t type : 2; 
uint8_t seqnum;
uint16_t length; 
uint32_t timestamp;
uint32_t crc1; 
char *payload;
uint32_t crc2;
};

pkt_t* pkt_new() {

pkt_t *new = (pkt_t *)malloc(sizeof(pkt_t));
new->type = PTYPE_DATA;
new->tr = 0;
new->window = 0;
new->seqnum = 0;
new->length = htons(0);
new->timestamp = 0;
new->crc1 = htonl(0);
new->crc2 = htonl(0);
new->payload=NULL;
return new;
}

void pkt_del(pkt_t *pkt)
{       
    if (pkt->payload != NULL){
        free(pkt->payload);
        free(pkt);
    }

}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt)
{
    if(len>528){
        return E_LENGTH;
    }
    //On encode le type
    ptypes_t type=data[0] >>6;
    pkt_status_code Fonctio=PKT_OK;
    Fonctio=pkt_set_type(pkt,type);
      
    //On encode le tr
    uint8_t tr=(data[0]<<2)>>6;
    Fonctio=pkt_set_tr(pkt,tr);
    
    //On encode la fenetre
    uint8_t window=data[0]&0b00011111 ;
    Fonctio=pkt_set_window(pkt,window);
    
    //on encode le Seqnum
    Fonctio=pkt_set_seqnum(pkt,data[1]);
    
    //on encode la taille
    uint16_t length;
    memcpy(&length, data+2*sizeof(uint8_t), sizeof(uint16_t));
    Fonctio=pkt_set_length(pkt,htons(length));
    
    //on encode timestamp
    uint32_t timestamp;
    memcpy(&timestamp, data+4*sizeof(uint8_t), sizeof(uint32_t));//tester sans les sizeof juste int et donc au lieu de 32, on mets 4
    Fonctio=pkt_set_timestamp(pkt,timestamp);
    
    // on decode CRC1
    uint32_t crc1 = ntohl(*((uint32_t *)(data + 8)));
    uint32_t new_crc1 = crc32(0L, Z_NULL, 0);
    new_crc1 = crc32(new_crc1,(const Bytef*) data, 8);
    if(crc1 != new_crc1)
        return E_CRC;
    Fonctio = pkt_set_crc1(pkt, crc1);
    //on decode payload
    if(!pkt_get_tr(pkt)&&pkt_get_length(pkt)>0){
         pkt->payload=(char *)malloc(sizeof(char)*pkt->length);
        if(pkt->payload==NULL){
            return E_NOMEM;
            
        }
       memcpy(pkt->payload, data+12, pkt->length);
    }
   
    // on decode CRC2
    uint32_t crc2 = ntohl(*((uint32_t *)(data + 12 + pkt_get_length(pkt))));
    const char *buf = pkt_get_payload(pkt);
    uint32_t new_crc2 = crc32(0L, Z_NULL, 0);
    new_crc2 = crc32(new_crc2,(const Bytef *) buf, pkt_get_length(pkt));
    if(crc2 != new_crc2){
        return E_CRC;
    }
    Fonctio = pkt_set_crc2(pkt, crc2);
    
    return Fonctio;
}
pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
    *len=0;
    memcpy(buf+*len, pkt, 2); //header
    *len+=2;
    uint16_t e= htons(pkt->length);
    memcpy(buf+*len,&(e), 2); //length
    *len+=2;
    memcpy(buf+*len,&(pkt->timestamp),4); //timestamp
    *len+=4;
    uint32_t crc = htonl(crc32(0, (Bytef*) buf, 8));//crc1
    memcpy(buf+*len,&crc,4);
    *len+=4;
    memcpy(buf+12, pkt->payload, pkt->length);
    *len+=pkt->length;
    if(pkt->length>0){
        crc = htonl(crc32(0, (Bytef*) pkt->payload, pkt->length));
        memcpy(buf+ *len, &crc, 4);
        *len+=4;
    }
    
 return PKT_OK;
}

ptypes_t pkt_get_type(const pkt_t* pkt)
{
return pkt->type;
}

uint8_t  pkt_get_tr(const pkt_t* pkt)
{
return pkt->tr;
}

uint8_t  pkt_get_window(const pkt_t* pkt)
{
return pkt->window;
}

uint8_t  pkt_get_seqnum(const pkt_t* pkt)
{
return pkt->seqnum;
}

uint16_t pkt_get_length(const pkt_t* pkt)
{
return pkt->length;
}

uint32_t pkt_get_timestamp(const pkt_t* pkt)
{
return pkt->timestamp ;
}

uint32_t pkt_get_crc1(const pkt_t* pkt)
{
return pkt->crc1;
}

uint32_t pkt_get_crc2(const pkt_t* pkt)
{
return pkt->crc2;
}

const char* pkt_get_payload(const pkt_t* pkt)
{
return pkt->payload;
}

pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
    if (type != 1 && type!=2 && type!=3){
        return E_TYPE;
    }
    pkt->type = type;
    return PKT_OK;
}

pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr)
{
    if (tr!= 1 && tr!=0){
        
        return E_TR;
    }
    pkt->tr = tr;
    return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
    if (window > MAX_WINDOW_SIZE){// essayer retirer
        printf("Erreur Set window >512");
        return E_WINDOW;
    }
    pkt->window = window;
    return PKT_OK;
}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
    pkt->seqnum = seqnum;
    return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
    if (length > MAX_PAYLOAD_SIZE){
        printf("Erreur Set Length ");
        return E_LENGTH;
    }
    pkt->length = length;
    return PKT_OK;
}

pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
    pkt->timestamp = timestamp ;
    return PKT_OK;
}

pkt_status_code pkt_set_crc1(pkt_t *pkt, const uint32_t crc1)
{
    pkt->crc1 = crc1;
    return PKT_OK;
}

pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
    pkt->crc2 = (crc2);
    return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt, const char *data, const uint16_t length)
{
    if (length>512){
        printf("Erreur Set Payload >512");
        return E_NOMEM;
    }
    if (pkt->payload!=NULL){
        
        free(pkt->payload);
    } 
    pkt->payload = (char *) malloc(length);
    if (pkt->payload == NULL){
        printf("Erreur Set Payload alloc");
        return E_NOMEM;
    } 
    memcpy(pkt->payload, data, length);
    pkt_set_length(pkt,length);
    return PKT_OK;
}
