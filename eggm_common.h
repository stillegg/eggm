
#if 0
void printEggmMsg( struct eggm_msg_t msg);

void printPacket(char *packet);

int makeEggmMsg(char msg_type, int src_id, int dst_id, int msg_id, char *data, int data_len, struct eggm_msg_t *msg);

int makeSendPacket ( struct eggm_msg_t send_msg, char* sendPacket );

int parseRecvPacket ( char *recvPacket, struct eggm_msg_t *recv_msg);
   
#endif
