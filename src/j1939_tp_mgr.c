/**
 * @file j1939_tp_mgr.c
 *
 * @brief
 */

#include <J1939/j1939_config.h>

#include <errno.h>

#include <J1939/j1939_bsp.h>

#include <J1939/private/j1939_private.h>
#include <J1939/private/j1939_tp_mgr.h>
#include <J1939/private/j1939_tp_mgr_msg.h>


///
#define J1939_TP_MGR_NO_ID                  ((uint8_t)0xFFU)


/**
 * @brief
 *
 * @param tp_mgr_ctx
 */
void j1939_tp_mgr_init(j1939_tp_mgr_ctx *const tp_mgr_ctx) {
    register int i;

    if (!tp_mgr_ctx) {
        return;
    }

    memset(tp_mgr_ctx, 0, sizeof(j1939_tp_mgr_ctx));

    memset(tp_mgr_ctx->bam_rx_tab, J1939_TP_MGR_NO_ID, sizeof(tp_mgr_ctx->bam_rx_tab));
    memset(tp_mgr_ctx->rts_rx_tab, J1939_TP_MGR_NO_ID, sizeof(tp_mgr_ctx->rts_rx_tab));
    memset(tp_mgr_ctx->xxx_tx_tab, J1939_TP_MGR_NO_ID, sizeof(tp_mgr_ctx->xxx_tx_tab));

    for (i = 0; i < J1939_TP_SESSIONS_NUM; ++i) {
        tp_mgr_ctx->sessions[i].id = i;
        tp_mgr_ctx->sessions[i].transmition_timeout = J1939_TP_TO_INF;
        tp_mgr_ctx->sessions[i].state = J1939_TP_STATE_FREE;
        tp_mgr_ctx->sessions[i].dir = J1939_TP_DIR_UNKNOWN;
    }
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 *
 * @return
 */
static int __get_free_tp_rx_session(j1939_tp_mgr_ctx *const tp_mgr_ctx) {
    register int i;

    for (i = 0; i < J1939_TP_RX_SESSIONS_NUM; ++i) {
        register j1939_tp_session *session = &tp_mgr_ctx->rx_sessions[i];

        if (session->state == J1939_TP_STATE_FREE) {
            session->state = J1939_TP_STATE_RESERVED;
            return session->id;
        }
    }

    return -ENOMEM;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param bam
 *
 * @return
 */
static int __get_free_tp_tx_session(j1939_tp_mgr_ctx *const tp_mgr_ctx, int bam) {
    register int i;

    if (bam) {
        register j1939_tp_session *session = &tp_mgr_ctx->tx_sessions[0];

        if (session->state == J1939_TP_STATE_FREE) {
            session->state = J1939_TP_STATE_RESERVED;
            return session->id;
        }
    } else {
        for (i = 1; i <= J1939_TP_TX_SESSIONS_NUM; ++i) {
            register j1939_tp_session *session = &tp_mgr_ctx->tx_sessions[i];

            if (session->state == J1939_TP_STATE_FREE) {
                session->state = J1939_TP_STATE_RESERVED;
                return session->id;
            }
        }
    }

    return -ENOMEM;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param session
 */
void __clean_tables(j1939_tp_mgr_ctx *const tp_mgr_ctx, j1939_tp_session *session) {
    if (!session) {
        return;
    }

    if (session->dir == J1939_TP_DIR_IN) {
        if (session->mode == J1939_TP_MODE_BAM) {
            tp_mgr_ctx->bam_rx_tab[session->src_addr] = J1939_TP_MGR_NO_ID;
        } else {
            tp_mgr_ctx->rts_rx_tab[session->src_addr] = J1939_TP_MGR_NO_ID;
        }
    } else if (session->dir == J1939_TP_DIR_OUT) {
        if (session->mode == J1939_TP_MODE_BAM) {
            tp_mgr_ctx->xxx_tx_tab[J1939_GLOBAL_ADDRESS] = J1939_TP_MGR_NO_ID;
        } else {
            tp_mgr_ctx->xxx_tx_tab[session->dst_addr] = J1939_TP_MGR_NO_ID;
        }
    }
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param sid
 *
 * @return
 */
int __detach_tp_session(j1939_tp_mgr_ctx *const tp_mgr_ctx, int sid) {
    j1939_tp_session *session;

    if ((uint32_t)(sid) >= J1939_TP_SESSIONS_NUM) {
        return -1;
    }

    session = &tp_mgr_ctx->sessions[sid];
    if (session->state == J1939_TP_STATE_FREE) {
        return -2;
    }

    session->state = J1939_TP_STATE_RESERVED;

    __clean_tables(tp_mgr_ctx, session);

    session->transmition_timeout = J1939_TP_TO_INF;
    session->state = J1939_TP_STATE_READY;

    return 0;
}


/**
 * @brief 
 * 
 * @param tp_mgr_ctx 
 * @param sid 
 * @return int 
 */
int __close_tp_session_with_error(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, int sid, j1939_rx_tx_errno error) {
    j1939_tp_session *session;

    if ((uint32_t)(sid) >= J1939_TP_SESSIONS_NUM) {
        return -1;
    }

    session = &tp_mgr_ctx->sessions[sid];
    if (session->state == J1939_TP_STATE_FREE) {
        return -2;
    }

    session->state = J1939_TP_STATE_RESERVED;
    session->transmition_timeout = J1939_TP_TO_INF;

    /* Error notification */
    if ((session->dir == J1939_TP_DIR_IN) && (error > J1939_RX_TX_ERROR_SUCCESS)) {
        __j1939_rx_error_notify(index, error, session->PGN, session->src_addr, session->msg_sz);
    } else if (session->dir == J1939_TP_DIR_OUT) {
        __j1939_tx_error_notify(index, error, session->PGN, session->dst_addr, session->msg_sz);
    }

    __clean_tables(tp_mgr_ctx, session);
    session->state = J1939_TP_STATE_FREE;

    return 0;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param sid
 *
 * @return
 */
int __close_tp_session(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, int sid) {
    return __close_tp_session_with_error(index, tp_mgr_ctx, sid, J1939_RX_TX_ERROR_SUCCESS);
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param tab
 * @param addr
 *
 * @return
 */
j1939_tp_session *__look_at_rx_table(j1939_tp_mgr_ctx *const tp_mgr_ctx, const uint8_t tab[], uint8_t addr) {
    uint8_t sid;

    sid = tab[addr];
    if (sid != J1939_TP_MGR_NO_ID) {
        return &tp_mgr_ctx->sessions[sid];
    }

    return NULL;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param tab
 * @param addr
 *
 * @return
 */
j1939_tp_session *__look_at_tx_table(j1939_tp_mgr_ctx *const tp_mgr_ctx, const uint8_t tab[], uint8_t addr) {
    uint8_t sid;

    sid = tab[addr];
    if (sid != J1939_TP_MGR_NO_ID) {
        return &tp_mgr_ctx->sessions[sid];
    }

    return NULL;
}


/**
 * @brief
 *
 * @param session
 * @param dir
 * @param src_address
 * @param tp_cm
 */
void __tp_session_setup_BAM(j1939_tp_session *const session, j1939_tp_session_dir dir, uint8_t src_address, const j1939_tp_cm_control *const tp_cm, uint32_t time) {
    session->dst_addr               = J1939_GLOBAL_ADDRESS;
    session->src_addr               = src_address;
    session->dir                    = dir;
    session->mode                   = J1939_TP_MODE_BAM;
    session->PGN                    = j1939_PGN_code_get(tp_cm->PGN);
    session->msg_sz                 = tp_cm->BAM.total_msg_sz;
    session->total_pkt_num          = tp_cm->BAM.total_pkt_num;
    session->pkt_max                = tp_cm->BAM.total_pkt_num;
    session->pkt_next               = 1;
    session->transmition_timeout    = (dir == J1939_TP_DIR_IN) ? J1939_TP_TO_T1 : J1939_TP_TO_INF;
    session->time                   = time;
}


/**
 * @brief
 *
 * @param session
 * @param dir
 * @param src_address
 * @param dst_address
 * @param tp_cm
 */
void __tp_session_setup_RTS(j1939_tp_session *const session, j1939_tp_session_dir dir, uint8_t src_address, uint8_t dst_address, const j1939_tp_cm_control *const tp_cm, uint32_t time) {
    session->dst_addr               = dst_address;
    session->src_addr               = src_address;
    session->dir                    = dir;
    session->mode                   = J1939_TP_MODE_RTS;
    session->PGN                    = j1939_PGN_code_get(tp_cm->PGN);
    session->msg_sz                 = tp_cm->RTS.total_msg_sz;
    session->total_pkt_num          = tp_cm->RTS.total_pkt_num;
    session->pkt_max                = U8_MIN(tp_cm->RTS.max_pkt_num, U8_MIN(J1939_TP_MGR_MAX_PACKETS_PER_CTS, tp_cm->RTS.total_pkt_num));
    session->pkt_next               = 1;
    session->transmition_timeout    = (dir == J1939_TP_DIR_IN) ? J1939_TP_TO_T2 : J1939_TP_TO_T3;
    session->time                   = time;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param src_address
 * @param control
 *
 * @return
 */
static int __open_rx_session(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t src_addr, uint8_t dst_addr, const j1939_tp_cm_control *const tp_cm, uint32_t time) {
    int sid;
    j1939_tp_session *session;

    if (!tp_mgr_ctx || !tp_cm || (tp_cm->control != J1939_TP_CM_BAM && tp_cm->control != J1939_TP_CM_RTS)) {
        return -EINVAL;
    }

    if ((tp_cm->control == J1939_TP_CM_BAM && tp_mgr_ctx->bam_rx_tab[src_addr] != J1939_TP_MGR_NO_ID) ||
        (tp_cm->control == J1939_TP_CM_RTS && tp_mgr_ctx->rts_rx_tab[src_addr] != J1939_TP_MGR_NO_ID)) {
        return -EISCONN;
    }

    sid = __get_free_tp_rx_session(tp_mgr_ctx);
    if (sid < 0) {
        return -ENOMEM;
    }

    session = &tp_mgr_ctx->sessions[sid];

    if (tp_cm->control == J1939_TP_CM_BAM) {
        __tp_session_setup_BAM(session, J1939_TP_DIR_IN, src_addr, tp_cm, time);
        tp_mgr_ctx->bam_rx_tab[src_addr] = sid;
    } else {
        __tp_session_setup_RTS(session, J1939_TP_DIR_IN, src_addr, dst_addr, tp_cm, time);
        tp_mgr_ctx->rts_rx_tab[src_addr] = sid;
    }

    session->state = J1939_TP_STATE_TRANSMIT;

    return sid;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param dst_addr
 * @param tp_cm
 *
 * @return
 */
static int __open_tx_session(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t dst_addr, const j1939_tp_cm_control *const tp_cm) {
    int sid;
    j1939_tp_session *session;
    uint8_t self_addr = j1939_get_address(index);

    if (!tp_mgr_ctx || !tp_cm ||
         (self_addr == J1939_NULL_ADDRESS) || (dst_addr == J1939_NULL_ADDRESS) ||
         (tp_cm->control != J1939_TP_CM_BAM && tp_cm->control != J1939_TP_CM_RTS)) {
        return -EINVAL;
    }

    if (tp_mgr_ctx->xxx_tx_tab[dst_addr] != J1939_TP_MGR_NO_ID) {
        return -EISCONN;
    }

    sid = __get_free_tp_tx_session(tp_mgr_ctx, (tp_cm->control == J1939_TP_CM_BAM));
    if (sid < 0) {
        return -ENOMEM;
    }

    session = &tp_mgr_ctx->sessions[sid];

    if (tp_cm->control == J1939_TP_CM_BAM) {
        __tp_session_setup_BAM(session, J1939_TP_DIR_OUT, self_addr, tp_cm, 0 /* on tx there is no time */);
        tp_mgr_ctx->xxx_tx_tab[J1939_GLOBAL_ADDRESS] = sid;
    } else {
        __tp_session_setup_RTS(session, J1939_TP_DIR_OUT, self_addr, dst_addr, tp_cm, 0 /* on tx there is no time */);
        tp_mgr_ctx->xxx_tx_tab[dst_addr] = sid;
    }

    return sid;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param SA
 * @param DA
 * @param tp_cm
 */
static void __tp_mgr_rx_handle_RTS_control(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_cm_control *const tp_cm, uint32_t time) {
    const uint32_t tp_cm_PGN = j1939_PGN_code_get(tp_cm->PGN);
    int sid;

    if ((sid = __open_rx_session(tp_mgr_ctx, SA, DA, tp_cm, time)) < 0) {
        j1939_rx_tx_errno rx_tx_error;
        j1939_tp_cm_conn_abort_reason reason;

        if (sid == -EISCONN) {
            reason = J1939_CONN_ABORT_REASON_EXISTS;
            rx_tx_error = J1939_RX_TX_ERROR_EXISTS;
        } else {
            reason = J1939_CONN_ABORT_REASON_NO_RESOURCES;
            rx_tx_error = J1939_RX_TX_ERROR_FAILED;
        }

        /* terminate connection */
        __send_Conn_Abort(index, DA, SA, tp_cm_PGN, reason); 
        __j1939_rx_error_notify(index, rx_tx_error, tp_cm_PGN, SA, 0);
    } else {
        const j1939_tp_session *const session = &tp_mgr_ctx->sessions[sid];

        /* establish connection */
        __send_CTS(index,
                   DA,
                   SA,
                   tp_cm_PGN,
                   session->pkt_max,
                   session->pkt_next);
    }
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param SA
 * @param DA
 * @param tp_cm
 */
static void __tp_mgr_rx_handle_BAM_control(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_cm_control *const tp_cm, uint32_t time) {
    int sid;

    /* opens BAM session */
    if ((sid = __open_rx_session(tp_mgr_ctx, SA, DA, tp_cm, time)) < 0) {
        const uint32_t tp_cm_PGN = j1939_PGN_code_get(tp_cm->PGN);
        /* no error check, just notify */
        __j1939_rx_error_notify(index, ((sid == -EISCONN) ? J1939_RX_TX_ERROR_EXISTS : J1939_RX_TX_ERROR_FAILED),
            tp_cm_PGN, SA, 0);
    }
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param SA
 * @param DA
 * @param tp_cm
 */
static void __tp_mgr_rx_handle_CTS_control(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_cm_control *const tp_cm) {
    j1939_tp_session *session = __look_at_tx_table(tp_mgr_ctx, tp_mgr_ctx->xxx_tx_tab, SA);

    (void)index;
    (void)DA;

    if (!session || !(session->state == J1939_TP_STATE_WAIT_CTS || session->state == J1939_TP_STATE_WAIT_EOMA)) {
        return;
    }

    session->transmition_timeout = J1939_TP_TO_INF;
    /* FIXME: what to do if destination sent wrong parameters? */
    session->pkt_max  = tp_cm->CTS.pkt_num;
    session->pkt_next = tp_cm->CTS.pkt_next;
    session->state = J1939_TP_STATE_TRANSMIT;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param SA
 * @param DA
 * @param tp_cm
 */
static void __tp_mgr_rx_handle_EoMA_control(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_cm_control *const tp_cm) {
    j1939_tp_session *session = __look_at_tx_table(tp_mgr_ctx, tp_mgr_ctx->xxx_tx_tab, SA);

    (void)DA;
    (void)tp_cm;

    if (!session || session->state != J1939_TP_STATE_WAIT_EOMA /* close session only in state WAIT_EOMA */) {
        return;
    }

    __close_tp_session(index, tp_mgr_ctx, session->id);
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param SA
 * @param DA
 * @param tp_cm
 */
static void __tp_mgr_rx_handle_Conn_Abort_control(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_cm_control *const tp_cm) {
    const uint32_t tp_cm_PGN = j1939_PGN_code_get(tp_cm->PGN);
    j1939_tp_session *session;

    (void)DA;

    /* try to close in session */
    session = __look_at_rx_table(tp_mgr_ctx, tp_mgr_ctx->rts_rx_tab, SA);
    if (session && session->state != J1939_TP_STATE_TIMEDOUT && session->PGN == tp_cm_PGN) {
        __close_tp_session_with_error(index, tp_mgr_ctx, session->id, J1939_RX_TX_ERROR_ABORTED);
        return;
    }

    /* try to close out session */
    session = __look_at_tx_table(tp_mgr_ctx, tp_mgr_ctx->xxx_tx_tab, SA);
    if (session && session->mode == J1939_TP_MODE_RTS) { /* originator can only close RTS */
        if (session->state != J1939_TP_STATE_TIMEDOUT && session->PGN == tp_cm_PGN) {
            __close_tp_session_with_error(index, tp_mgr_ctx, session->id, J1939_RX_TX_ERROR_ABORTED);
            return;
        }
    }
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param SA
 * @param DA
 * @param tp_dt
 */
static void __tp_mgr_rx_handle_BAM_DT_transmition(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_dt *const tp_dt) {
    j1939_tp_session *const session = __look_at_rx_table(tp_mgr_ctx, tp_mgr_ctx->bam_rx_tab, SA);

    (void)DA;

    if (!session || session->state != J1939_TP_STATE_TRANSMIT) {
        return;
    }

    /* have we lost a packet? */
    if (session->pkt_next != tp_dt->seq_num) {
        /* if so, close the session */
        __close_tp_session_with_error(index, tp_mgr_ctx, session->id, J1939_RX_TX_ERROR_LOST_PACKET);
        return;
    }

    /* copy received data into dedicated buffer */
    memcpy(&session->buffer[(session->pkt_next - 1) * J1939_MULTIPACKET_DATA_SZ],
           tp_dt->payload,
           J1939_MULTIPACKET_DATA_SZ);

    /* is this a last packet? */
    if (session->pkt_next == session->total_pkt_num) {
        /* append received data into fifo */
        int status = __j1939_receive_notify(index,
                                            J1939_RX_INFO_TYPE_MULTIPACKET | session->id,
                                            session->PGN,
                                            session->src_addr,
                                            session->dst_addr,
                                            session->msg_sz,
                                            session->buffer,
                                            session->time);
        if (status < 0) {
            /* if we cannot append a message just close */
            __close_tp_session_with_error(index, tp_mgr_ctx, session->id, J1939_RX_TX_ERROR_FAILED);
        } else {
            /* detach session to free broadcast connection with src addr */
            __detach_tp_session(tp_mgr_ctx, session->id);
        }
    } else {
        /* try receive the next packet */
        session->transmition_timeout = J1939_TP_TO_T1;
        session->pkt_next++;
    }
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param SA
 * @param DA
 * @param tp_dt
 */
static void __tp_mgr_rx_handle_RTS_DT_transmition(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_dt *const tp_dt) {
    j1939_tp_session *const session = __look_at_rx_table(tp_mgr_ctx, tp_mgr_ctx->rts_rx_tab, SA);

    if (!session || session->state != J1939_TP_STATE_TRANSMIT) {
        return;
    }

    /* have we lost a packet? */
    if (session->pkt_next != tp_dt->seq_num) {
        /* if so, abort the session */
        __send_Conn_Abort(index, DA, SA, session->PGN, J1939_CONN_ABORT_REASON_NO_RESOURCES);
        __close_tp_session_with_error(index, tp_mgr_ctx, session->id, J1939_RX_TX_ERROR_LOST_PACKET);
        return;
    }

    /* copy received data into dedicated buffer */
    memcpy(&session->buffer[(session->pkt_next - 1) * J1939_MULTIPACKET_DATA_SZ],
           tp_dt->payload,
           J1939_MULTIPACKET_DATA_SZ);

    if (session->pkt_next == session->total_pkt_num) {
        /* append received data into fifo */
        int status = __j1939_receive_notify(index,
                                            J1939_RX_INFO_TYPE_MULTIPACKET | session->id,
                                            session->PGN,
                                            session->src_addr,
                                            session->dst_addr,
                                            session->msg_sz,
                                            session->buffer,
                                            session->time);
        if (status < 0) {
            /* if we cannot append a message just abort transmition */
            __send_Conn_Abort(index, DA, SA, session->PGN, J1939_CONN_ABORT_REASON_NO_RESOURCES);
            __close_tp_session_with_error(index, tp_mgr_ctx, session->id, J1939_RX_TX_ERROR_FAILED);
        } else {
            /* acknowledge receiving a message */
            __send_EoMA(index, DA, SA, session->PGN, session->msg_sz, session->pkt_next);
            __detach_tp_session(tp_mgr_ctx, session->id);
        }
    } else if (0 == (session->pkt_next % session->pkt_max)) {
        /* update a max number of receiving packets */
        /* SAE J1939-21:
         * This value shall be no larger than the value in byte 5 of the RTS message. */
        session->pkt_max = U8_MIN(session->pkt_max, (session->total_pkt_num - session->pkt_next));

        /* try receive the next packet */
        session->transmition_timeout = J1939_TP_TO_T2;
        session->pkt_next++;

        /* acknowledge to receive the next set of packets */
        __send_CTS(index, DA, SA, session->PGN, session->pkt_max, session->pkt_next);
    } else {
        /* try receive the next packet */
        session->transmition_timeout = J1939_TP_TO_T1;
        session->pkt_next++;
    }
}


/**
 * @brief
 *
 * @param time_ms
 * @param frame
 */
int j1939_tp_mgr_rx_handler(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, const j1939_primitive *const frame, uint32_t time) {
    const uint32_t PGN = j1939_PGN_code_get(frame->PGN);
    uint8_t SA;
    uint8_t DA;
    int is_TP_CM = (PGN == J1939_STD_PGN_TPCM) && (frame->dlc == J1939_STD_PGN_TPCM_DLC);
    int is_TP_DT = (PGN == J1939_STD_PGN_TPDT) && (frame->dlc == J1939_STD_PGN_TPDT_DLC);
    int level;

    if (!is_TP_CM && !is_TP_DT) {
        return 0;
    }

    SA = frame->src_address;
    DA = frame->PGN.dest_address;

    /* don't handle frame if dest.address is not global or is not mine */
    if (DA != J1939_GLOBAL_ADDRESS && DA != j1939_get_address(index)) {
        return 1;
    }

    /* there isn't receiving at the moment */
    if (is_TP_DT && (tp_mgr_ctx->bam_rx_tab[SA] == J1939_TP_MGR_NO_ID && tp_mgr_ctx->rts_rx_tab[SA] == J1939_TP_MGR_NO_ID)) {
        return 1;
    }

    level = j1939_bsp_lock();

    if (is_TP_CM) {
        /* handle TP Connection Management frame */
        const j1939_tp_cm_control *const tp_cm = (j1939_tp_cm_control*)&frame->payload[0];

        switch (tp_cm->control) {
            case J1939_TP_CM_RTS:
                if (DA != J1939_GLOBAL_ADDRESS) {
                    __tp_mgr_rx_handle_RTS_control(index, tp_mgr_ctx, SA, DA, tp_cm, time);
                }
                break;

            case J1939_TP_CM_BAM:
                if (DA == J1939_GLOBAL_ADDRESS) {
                    __tp_mgr_rx_handle_BAM_control(index, tp_mgr_ctx, SA, DA, tp_cm, time);
                }
                break;

            case J1939_TP_CM_CTS:
                if (DA != J1939_GLOBAL_ADDRESS) {
                    __tp_mgr_rx_handle_CTS_control(index, tp_mgr_ctx, SA, DA, tp_cm);
                }
                break;

            case J1939_TP_CM_EndOfMsgACK:
                if (DA != J1939_GLOBAL_ADDRESS) {
                    __tp_mgr_rx_handle_EoMA_control(index, tp_mgr_ctx, SA, DA, tp_cm);
                }
                break;

            case J1939_TP_CM_Conn_Abort:
                if (DA != J1939_GLOBAL_ADDRESS) {
                    __tp_mgr_rx_handle_Conn_Abort_control(index, tp_mgr_ctx, SA, DA, tp_cm);
                }
                break;

            default:
                j1939_bsp_unlock(level);
                return 1;
        }
    } else {
        /* handle TP Data frame */
        const j1939_tp_dt *const tp_dt = (j1939_tp_dt*)&frame->payload[0];

        if (DA == J1939_GLOBAL_ADDRESS) {
            /* handle BAM Data frames*/
            __tp_mgr_rx_handle_BAM_DT_transmition(index, tp_mgr_ctx, SA, DA, tp_dt);
        } else {
            /* handle RTS Data frames */
            __tp_mgr_rx_handle_RTS_DT_transmition(index, tp_mgr_ctx, SA, DA, tp_dt);
        }
    }

    j1939_bsp_unlock(level);

    return 1;
}


/**
 * @brief
 *
 * @param session
 * @param t_delta
 *
 * @return
 */
static int __tp_mgr_session_timeout_check(j1939_tp_session *const session, uint32_t t_delta) {
    int sts;
    int level = j1939_bsp_lock();

    if (session->transmition_timeout != J1939_TP_TO_INF) {
        session->transmition_timeout -= t_delta;
    }

    if (session->transmition_timeout < 0) {
        session->state = J1939_TP_STATE_TIMEDOUT;
        sts = 1;
    } else {
        sts = 0;
    }

    j1939_bsp_unlock(level);

    return sts;
}


/**
 * @brief
 *
 * @param self_addr
 * @param tp_mgr_ctx
 * @param session
 * @param t_delta
 *
 * @return
 */
static int __tp_mgr_process_timeout_check(uint8_t index, uint8_t self_addr, j1939_tp_mgr_ctx *const tp_mgr_ctx, j1939_tp_session *const session, uint32_t t_delta) {
    int is_timedout;

    if (session->state != J1939_TP_STATE_TRANSMIT &&
        session->state != J1939_TP_STATE_WAIT_CTS &&
        session->state != J1939_TP_STATE_WAIT_EOMA) {
        return 0;
    }

    is_timedout = __tp_mgr_session_timeout_check(session, t_delta);

    if (is_timedout) {
        if (session->mode == J1939_TP_MODE_RTS) {
            uint8_t dst_addr = (session->dir == J1939_TP_DIR_IN) ? session->src_addr : session->dst_addr;
            __send_Conn_Abort(index, self_addr, dst_addr, session->PGN, J1939_CONN_ABORT_REASON_TIMEDOUT);
        }
        __close_tp_session_with_error(index, tp_mgr_ctx, session->id, J1939_RX_TX_ERROR_TIMEDOUT);
    }

    return is_timedout;
}


/**
 * @brief
 *
 * @param self_addr
 * @param tp_mgr_ctx
 * @param session
 *
 * @return
 */
static int __tp_mgr_process_transmition(uint8_t index, uint8_t self_addr, j1939_tp_mgr_ctx *const tp_mgr_ctx, j1939_tp_session *const session) {
    j1939_tp_dt tp_dt;
    unsigned msg_start;
    unsigned msg_sz_min;
    int level;
    int is_active;

    if (session->state != J1939_TP_STATE_TRANSMIT || session->dir != J1939_TP_DIR_OUT) {
        return 0;
    }

    /* receiver holded the transmition */
    if (session->pkt_max == 0) {
        session->transmition_timeout = J1939_TP_TO_T4;
        session->state = J1939_TP_STATE_WAIT_CTS;
        return 1;
    }

    memset(&tp_dt, 0xFF, sizeof(j1939_tp_dt));

    msg_start  = (session->pkt_next - 1) * J1939_MULTIPACKET_DATA_SZ;
    msg_sz_min = U16_MIN(J1939_MULTIPACKET_DATA_SZ, (session->msg_sz - msg_start));

    /* copy transmition data into payload */
    memcpy(tp_dt.payload, &session->buffer[msg_start], msg_sz_min);
    tp_dt.seq_num = session->pkt_next;

    level = j1939_bsp_lock();

    if (__send_TPDT(index, self_addr, session->dst_addr, &tp_dt) < 0) {
        if (session->mode == J1939_TP_MODE_RTS) {
            __send_Conn_Abort(index, self_addr, session->dst_addr, session->PGN, J1939_CONN_ABORT_REASON_NO_RESOURCES);
        }
        __close_tp_session_with_error(index, tp_mgr_ctx, session->id, J1939_RX_TX_ERROR_FAILED);
        j1939_bsp_unlock(level);
        return 0;
    }

    is_active = 1;

    if (session->mode == J1939_TP_MODE_RTS) {
        if ((--session->pkt_max == 0) && (session->pkt_next < session->total_pkt_num)) {
            session->transmition_timeout = J1939_TP_TO_T3;
            session->state = J1939_TP_STATE_WAIT_CTS;
        } else if (session->pkt_next == session->total_pkt_num) {
            session->transmition_timeout = J1939_TP_TO_T3;
            session->state = J1939_TP_STATE_WAIT_EOMA;
        } else {
            /* transmit next packet */
            session->transmition_timeout = J1939_TP_TO_INF;
            session->pkt_next++;
        }
    } else /* BAM mode */ {
        if (session->pkt_next == session->total_pkt_num) {
            /* we have sent the last frame, close the session */
            __close_tp_session(index, tp_mgr_ctx, session->id);
            is_active = 0;
        } else {
            /* transmit next packet */
            session->transmition_timeout = J1939_TP_TO_INF;
            session->pkt_next++;
        }
    }

    j1939_bsp_unlock(level);

    return is_active;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param time_ms
 */
int j1939_tp_mgr_process(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint32_t t_delta) {
    register int i;
    int level;
    uint8_t CA_addr = j1939_get_address(index);
    int activities;

    if (tp_mgr_ctx->reset) {
        level = j1939_bsp_lock();
        j1939_tp_mgr_init(tp_mgr_ctx);
        j1939_bsp_unlock(level);
        return 1;
    }

    if (CA_addr == J1939_NULL_ADDRESS) {
        return 0;
    }

    activities = 0;

    /* process timeout functionality */
    for (i = 0; i < J1939_TP_SESSIONS_NUM; ++i) {
        j1939_tp_session *const session = &tp_mgr_ctx->sessions[i];
        activities += (session->state != J1939_TP_STATE_FREE);
        __tp_mgr_process_timeout_check(index, CA_addr, tp_mgr_ctx, session, t_delta);
    }

    /* process RTS & BAM transmition functionality */
    for (i = 0; i <= J1939_TP_TX_SESSIONS_NUM; ++i) {
        /* for "<=" operation see tx_sessions definition */
        j1939_tp_session *const session = &tp_mgr_ctx->tx_sessions[i];
        activities += __tp_mgr_process_transmition(index, CA_addr, tp_mgr_ctx, session);
    }

    return (activities > 0);
}


/**
 * @brief
 *
 * @return
 */
int j1939_tp_mgr_open_tx_session(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload) {
    int sid;
    j1939_tp_session *session;
    j1939_tp_cm_control tp_cm_control;
    int level;
    unsigned pkt_num;

    if (dst_addr == J1939_NULL_ADDRESS || msg_sz > J1939_MAX_DATA_SZ) {
        return -EINVAL;
    }

    /* number of packets should be 2 or more */
    pkt_num = (msg_sz - 1) / J1939_MULTIPACKET_DATA_SZ + 1;
    if (pkt_num < 2) {
        return -EINVAL;
    }

    if (dst_addr == J1939_GLOBAL_ADDRESS) {
        tp_cm_control = __new_tp_cm_BAM(msg_sz, pkt_num, PGN);
    } else {
        tp_cm_control = __new_tp_cm_RTS(msg_sz,
                                        pkt_num,
                                        U8_MIN(pkt_num, J1939_TP_MGR_MAX_PACKETS_PER_CTS),
                                        PGN);
    }

    level = j1939_bsp_lock();
    sid = __open_tx_session(index, tp_mgr_ctx, dst_addr, &tp_cm_control);
    j1939_bsp_unlock(level);

    if (sid < 0) {
        return -ENOMEM;
    }

    session = &tp_mgr_ctx->sessions[sid];

    /* copy data to dedicated buffer */
    memcpy(session->buffer, payload, msg_sz * sizeof(uint8_t));

    level = j1939_bsp_lock();

    if (__send_TPCM(index, session->src_addr, session->dst_addr, &tp_cm_control) < 0) {
        __close_tp_session_with_error(index, tp_mgr_ctx, sid, J1939_RX_TX_ERROR_FAILED);
        j1939_bsp_unlock(level);
        return -EIO;
    }

    barrier();

    session->state = (session->mode == J1939_TP_MODE_BAM) ?
                     J1939_TP_STATE_TRANSMIT :
                     J1939_TP_STATE_WAIT_CTS;

    j1939_bsp_unlock(level);

    return 0;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param sid
 *
 * @return
 */
int j1939_tp_mgr_close_session(uint8_t index, j1939_tp_mgr_ctx *const tp_mgr_ctx, int sid) {
    int level = j1939_bsp_lock();
    int status = __close_tp_session(index, tp_mgr_ctx, sid);
    j1939_bsp_unlock(level);
    return status;
}
