/**
 * @file j1939_tp_mgr.c
 *
 * @brief
 */


#include <errno.h>

#include <J1939/j1939_bsp.h>

#include <J1939/private/j1939_private.h>
#include <J1939/private/j1939_tp_mgr.h>
#include <J1939/private/j1939_tp_mgr_msg.h>


///
#define J1939_TP_MGR_MAX_PACKETS_PER_CTS    ((uint8_t)8U)


/**
 * @brief
 *
 * @param tp_mgr_ctx
 */
void j1939_tp_mgr_init(j1939_tp_mgr_ctx *const tp_mgr_ctx) {
    register int i;

    if (!tp_mgr_ctx)
        return;

    memset(tp_mgr_ctx, 0, sizeof(j1939_tp_mgr_ctx));

    memset(tp_mgr_ctx->bam_rx_tab, 0xFFU, sizeof(tp_mgr_ctx->bam_rx_tab));
    memset(tp_mgr_ctx->rts_rx_tab, 0xFFU, sizeof(tp_mgr_ctx->rts_rx_tab));

    tp_mgr_ctx->bam_tx_tab[0] = 0xFFU;
    tp_mgr_ctx->rts_tx_tab[0] = 0xFFU;

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
int __get_free_tp_session(j1939_tp_mgr_ctx *const tp_mgr_ctx) {
    register int i;

    for (i = 0; i < J1939_TP_SESSIONS_NUM; ++i) {
        register j1939_tp_session *session = &tp_mgr_ctx->sessions[i];

        if (session->state == J1939_TP_STATE_FREE) {
            session->state = J1939_TP_STATE_RESERVED;
            return i;
        }
    }

    return -1;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param session
 */
void __clean_tables(j1939_tp_mgr_ctx *const tp_mgr_ctx, j1939_tp_session *session) {
    if (!session)
        return;

    if (session->dir == J1939_TP_DIR_IN) {
        if (session->mode == J1939_TP_MODE_BAM) {
            tp_mgr_ctx->bam_rx_tab[session->src_addr] = 255;
        } else {
            tp_mgr_ctx->rts_rx_tab[session->src_addr] = 255;
        }
    } else if (session->dir == J1939_TP_DIR_OUT) {
        if (session->mode == J1939_TP_MODE_BAM) {
            tp_mgr_ctx->bam_tx_tab[0] = 255;
        } else {
            tp_mgr_ctx->rts_tx_tab[0] = 255;
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

    if ((uint32_t)(sid) >= J1939_TP_SESSIONS_NUM)
        return -1;

    session = &tp_mgr_ctx->sessions[sid];
    if (session->state == J1939_TP_STATE_FREE)
        return -2;

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
 *
 * @return
 */
int __close_tp_session(j1939_tp_mgr_ctx *const tp_mgr_ctx, int sid) {
    j1939_tp_session *session;

    if ((uint32_t)(sid) >= J1939_TP_SESSIONS_NUM)
        return -1;

    session = &tp_mgr_ctx->sessions[sid];
    if (session->state == J1939_TP_STATE_FREE)
        return -2;

    session->state = J1939_TP_STATE_RESERVED;

    __clean_tables(tp_mgr_ctx, session);

    session->transmition_timeout = J1939_TP_TO_INF;
    session->state = J1939_TP_STATE_FREE;

    return 0;
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
    if (sid != 255)
        return &tp_mgr_ctx->sessions[sid];

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

    /* look at receiving table */
    sid = tab[0];
    if (sid != 255 && addr == tp_mgr_ctx->sessions[sid].dst_addr)
        return &tp_mgr_ctx->sessions[sid];

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
void __tp_session_setup_BAM(j1939_tp_session *const session, j1939_tp_session_dir dir, uint8_t src_address, const j1939_tp_cm_control *const tp_cm) {
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
void __tp_session_setup_RTS(j1939_tp_session *const session, j1939_tp_session_dir dir, uint8_t src_address, uint8_t dst_address, const j1939_tp_cm_control *const tp_cm) {
    session->dst_addr               = dst_address;
    session->src_addr               = src_address;
    session->dir                    = dir;
    session->mode                   = J1939_TP_MODE_RTS;
    session->PGN                    = j1939_PGN_code_get(tp_cm->PGN);
    session->msg_sz                 = tp_cm->RTS.total_msg_sz;
    session->total_pkt_num          = tp_cm->RTS.total_pkt_num;
    session->pkt_max                = U8_MIN(tp_cm->RTS.max_pkt_num, J1939_TP_MGR_MAX_PACKETS_PER_CTS);
    session->pkt_next               = 1;
    session->transmition_timeout    = (dir == J1939_TP_DIR_IN) ? J1939_TP_TO_T2 : J1939_TP_TO_T3;
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
static int __open_rx_session(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t src_addr, uint8_t dst_addr, const j1939_tp_cm_control *const tp_cm) {
    int sid;
    j1939_tp_session *session;

    if (!tp_mgr_ctx || !tp_cm || tp_cm->control != J1939_TP_CM_BAM || tp_cm->control != J1939_TP_CM_RTS)
        return -EINVAL;

    if ((tp_cm->control == J1939_TP_CM_BAM && tp_mgr_ctx->bam_rx_tab[src_addr] != 255) ||
        (tp_cm->control == J1939_TP_CM_RTS && tp_mgr_ctx->rts_rx_tab[src_addr] != 255))
        return -EISCONN;

    sid = __get_free_tp_session(tp_mgr_ctx);
    if (sid < 0)
        return -ENOMEM;

    session = &tp_mgr_ctx->sessions[sid];

    if (tp_cm->control == J1939_TP_CM_BAM) {
        __tp_session_setup_BAM(session, J1939_TP_DIR_IN, src_addr, tp_cm);
        tp_mgr_ctx->bam_rx_tab[src_addr] = sid;
    } else {
        __tp_session_setup_RTS(session, J1939_TP_DIR_IN, src_addr, dst_addr, tp_cm);
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
static int __open_tx_session(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t dst_addr, const j1939_tp_cm_control *const tp_cm) {
    int sid;
    j1939_tp_session *session;
    uint8_t self_addr = j1939_get_address();

    if (!tp_mgr_ctx || !tp_cm ||
         self_addr == J1939_NULL_ADDRESS || dst_addr == J1939_NULL_ADDRESS ||
         tp_cm->control != J1939_TP_CM_BAM || tp_cm->control != J1939_TP_CM_RTS)
        return -EINVAL;

    if ((tp_cm->control == J1939_TP_CM_BAM && tp_mgr_ctx->bam_tx_tab[0] != 255) ||
        (tp_cm->control == J1939_TP_CM_RTS && tp_mgr_ctx->rts_tx_tab[0] != 255))
        return -EISCONN;

    sid = __get_free_tp_session(tp_mgr_ctx);
    if (sid < 0)
        return -ENOMEM;

    session = &tp_mgr_ctx->sessions[sid];

    if (tp_cm->control == J1939_TP_CM_BAM) {
        __tp_session_setup_BAM(session, J1939_TP_DIR_OUT, self_addr, tp_cm);
        tp_mgr_ctx->bam_tx_tab[0] = sid;
    } else {
        __tp_session_setup_RTS(session, J1939_TP_DIR_OUT, self_addr, dst_addr, tp_cm);
        tp_mgr_ctx->rts_tx_tab[0] = sid;
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
static void __tp_mgr_rx_handle_RTS_control(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_cm_control *const tp_cm) {
    const uint32_t tp_cm_PGN = j1939_PGN_code_get(tp_cm->PGN);
    int sid;

    if ((sid = __open_rx_session(tp_mgr_ctx, SA, DA, tp_cm)) < 0) {
        /* terminate connection */
        __send_Conn_Abort(DA,
                          SA,
                          tp_cm_PGN,
                          (sid == -EISCONN) ?
                              J1939_CONN_ABORT_REASON_EXISTS :
                              J1939_CONN_ABORT_REASON_NO_RESOURCES);
    } else {
        const j1939_tp_session *const session = &tp_mgr_ctx->sessions[sid];

        /* establish connection */
        __send_CTS(DA,
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
static void __tp_mgr_rx_handle_BAM_control(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_cm_control *const tp_cm) {
    /* opens BAM session */
    (void)__open_rx_session(tp_mgr_ctx, SA, DA, tp_cm);
    /* no error check, just skip */
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param SA
 * @param DA
 * @param tp_cm
 */
static void __tp_mgr_rx_handle_CTS_control(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_cm_control *const tp_cm) {
    j1939_tp_session *session = __look_at_tx_table(tp_mgr_ctx, tp_mgr_ctx->rts_rx_tab, SA);

    if (!session || session->state != J1939_TP_STATE_WAIT_CTS)
        return;

    session->transmition_timeout = J1939_TP_TO_INF;
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
static void __tp_mgr_rx_handle_EoMA_control(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_cm_control *const tp_cm) {
    j1939_tp_session *session = __look_at_tx_table(tp_mgr_ctx, tp_mgr_ctx->rts_rx_tab, SA);

    if (!session)
        return;

    __close_tp_session(tp_mgr_ctx, session->id);
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param SA
 * @param DA
 * @param tp_cm
 */
static void __tp_mgr_rx_handle_Conn_Abort_control(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_cm_control *const tp_cm) {
    const uint32_t tp_cm_PGN = j1939_PGN_code_get(tp_cm->PGN);
    j1939_tp_session *session;

    /* try to close in session */
    session = __look_at_rx_table(tp_mgr_ctx, tp_mgr_ctx->rts_rx_tab, SA);
    if (session && session->state != J1939_TP_STATE_TIMEDOUT && session->PGN == tp_cm_PGN) {
        __close_tp_session(tp_mgr_ctx, session->id);
        return;
    }

    /* try to close out session */
    session = __look_at_tx_table(tp_mgr_ctx, tp_mgr_ctx->rts_tx_tab, SA);
    if (session && session->state != J1939_TP_STATE_TIMEDOUT && session->PGN == tp_cm_PGN) {
        __close_tp_session(tp_mgr_ctx, session->id);
        return;
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
static void __tp_mgr_rx_handle_BAM_DT_transmition(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_dt *const tp_dt) {
    j1939_tp_session *const session = __look_at_rx_table(tp_mgr_ctx, tp_mgr_ctx->bam_rx_tab, SA);

    if (!session || session->state != J1939_TP_STATE_TRANSMIT)
        return;

    /* have we lost a packet? */
    if (session->pkt_next != tp_dt->seq_num) {
        /* if so, close the session */
        __close_tp_session(tp_mgr_ctx, session->id);
        return;
    }

    /* copy received data into dedicated buffer */
    memcpy(&session->buffer[(session->pkt_next - 1) * J1939_MULTIPACKET_DATA_SZ],
           tp_dt->payload,
           J1939_MULTIPACKET_DATA_SZ);

    /* is this a last packet? */
    if (session->pkt_next == session->total_pkt_num) {
        /* append received data into fifo */
        int status = __j1939_receive_notify(J1939_RX_INFO_TYPE_MULTIPACKET | session->id,
                                            session->PGN,
                                            session->src_addr,
                                            session->msg_sz,
                                            session->buffer);
        if (status < 0) {
            /* if we cannot append a message just close */
            __close_tp_session(tp_mgr_ctx, session->id);
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
static void __tp_mgr_rx_handle_RTS_DT_transmition(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint8_t SA, uint8_t DA, const j1939_tp_dt *const tp_dt) {
    j1939_tp_session *const session = __look_at_rx_table(tp_mgr_ctx, tp_mgr_ctx->rts_rx_tab, SA);

    if (!session || session->state != J1939_TP_STATE_TRANSMIT)
        return;

    /* have we lost a packet? */
    if (session->pkt_next != tp_dt->seq_num) {
        /* if so, abort the session */
        __send_Conn_Abort(DA, SA, session->PGN, J1939_CONN_ABORT_REASON_NO_RESOURCES);
        __close_tp_session(tp_mgr_ctx, session->id);
        return;
    }

    /* copy received data into dedicated buffer */
    memcpy(&session->buffer[(session->pkt_next - 1) * J1939_MULTIPACKET_DATA_SZ],
           tp_dt->payload,
           J1939_MULTIPACKET_DATA_SZ);

    if (session->pkt_next == session->total_pkt_num) {
        /* append received data into fifo */
        int status = __j1939_receive_notify(J1939_RX_INFO_TYPE_MULTIPACKET | session->id,
                                            session->PGN,
                                            session->src_addr,
                                            session->msg_sz,
                                            session->buffer);
        if (status < 0) {
            /* if we cannot append a message just abort transmition */
            __send_Conn_Abort(DA, SA, session->PGN, J1939_CONN_ABORT_REASON_NO_RESOURCES);
            __close_tp_session(tp_mgr_ctx, session->id);
        } else {
            /* acknowledge receiving a message */
            __send_EoMA(DA, SA, session->PGN, session->msg_sz, session->pkt_next);
            __detach_tp_session(tp_mgr_ctx, session->id);
        }
    } else if (0 == (session->pkt_next % session->pkt_max)) {
        /* update a max number of receiving packets */
        session->pkt_max = U8_MIN(J1939_TP_MGR_MAX_PACKETS_PER_CTS, (session->total_pkt_num - session->pkt_next));

        /* try receive the next packet */
        session->transmition_timeout = J1939_TP_TO_T2;
        session->pkt_next++;

        /* acknowledge to receive the next set of packets */
        __send_CTS(DA, SA, session->PGN, session->pkt_max, session->pkt_next);
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
int j1939_tp_mgr_rx_handler(j1939_tp_mgr_ctx *const tp_mgr_ctx, const j1939_primitive *const frame) {
    const uint32_t PGN = j1939_PGN_code_get(frame->PGN);
    uint8_t SA;
    uint8_t DA;
    int is_TP_CM = (PGN == J1939_STD_PGN_TPCM) && (frame->dlc == J1939_STD_PGN_TPCM_DLC);
    int is_TP_DT = (PGN == J1939_STD_PGN_TPDT) && (frame->dlc == J1939_STD_PGN_TPDT_DLC);

    if (!is_TP_CM || !is_TP_DT)
        return 0;

    SA = frame->src_address;
    DA = frame->PGN.dest_address;

    /* don't handle frame if dest.address is not global or is not mine */
    if (DA != J1939_GLOBAL_ADDRESS || DA != j1939_get_address())
        return 1;

    /* there isn't receiving at the moment */
    if (is_TP_DT && (tp_mgr_ctx->bam_rx_tab[SA] == 255 && tp_mgr_ctx->rts_rx_tab[SA] == 255))
        return 1;

    if (is_TP_CM) {
        /* handle TP Connection Management frame */
        const j1939_tp_cm_control *const tp_cm = (j1939_tp_cm_control*)&frame->payload[0];

        switch (tp_cm->control) {
            case J1939_TP_CM_RTS:
                if (DA != J1939_GLOBAL_ADDRESS)
                    __tp_mgr_rx_handle_RTS_control(tp_mgr_ctx, SA, DA, tp_cm);
                break;

            case J1939_TP_CM_BAM:
                if (DA == J1939_GLOBAL_ADDRESS)
                    __tp_mgr_rx_handle_BAM_control(tp_mgr_ctx, SA, DA, tp_cm);
                break;

            case J1939_TP_CM_CTS:
                if (DA != J1939_GLOBAL_ADDRESS)
                    __tp_mgr_rx_handle_CTS_control(tp_mgr_ctx, SA, DA, tp_cm);
                break;

            case J1939_TP_CM_EndOfMsgACK:
                if (DA != J1939_GLOBAL_ADDRESS)
                    __tp_mgr_rx_handle_EoMA_control(tp_mgr_ctx, SA, DA, tp_cm);
                break;

            case J1939_TP_CM_Conn_Abort:
                if (DA != J1939_GLOBAL_ADDRESS)
                    __tp_mgr_rx_handle_Conn_Abort_control(tp_mgr_ctx, SA, DA, tp_cm);
                break;

            default:
                return 1;
        }
    } else {
        /* handle TP Data frame */
        const j1939_tp_dt *const tp_dt = (j1939_tp_dt*)&frame->payload[0];

        if (DA == J1939_GLOBAL_ADDRESS) {
            /* handle BAM Data frames*/
            __tp_mgr_rx_handle_BAM_DT_transmition(tp_mgr_ctx, SA, DA, tp_dt);
        } else {
            /* handle RTS Data frames */
            __tp_mgr_rx_handle_RTS_DT_transmition(tp_mgr_ctx, SA, DA, tp_dt);
        }
    }

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

    if (session->transmition_timeout != J1939_TP_TO_INF)
        session->transmition_timeout -= t_delta;

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
static int __tp_mgr_process_timeout_check(uint8_t self_addr, j1939_tp_mgr_ctx *const tp_mgr_ctx, j1939_tp_session *const session, uint32_t t_delta) {
    int is_timedout;

    if (session->state != J1939_TP_STATE_TRANSMIT &&
        session->state != J1939_TP_STATE_WAIT_CTS &&
        session->state != J1939_TP_STATE_WAIT_EOMA)
        return 0;

    is_timedout = __tp_mgr_session_timeout_check(session, t_delta);

    if (is_timedout) {
        if (session->mode == J1939_TP_MODE_RTS) {
            uint8_t dst_addr = (session->dir == J1939_TP_DIR_IN) ? session->src_addr : session->dst_addr;
            __send_Conn_Abort(self_addr, dst_addr, session->PGN, J1939_CONN_ABORT_REASON_TIMEDOUT);
        }
        __close_tp_session(tp_mgr_ctx, session->id);
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
static int __tp_mgr_process_transmition(uint8_t self_addr, j1939_tp_mgr_ctx *const tp_mgr_ctx, j1939_tp_session *const session) {
    j1939_tp_dt tp_dt;
    unsigned msg_start;
    unsigned msg_sz_min;
    int level;

    if (session->state != J1939_TP_STATE_TRANSMIT || session->dir != J1939_TP_DIR_OUT)
        return 0;

    /* receiver holded the transmition */
    if (session->pkt_max == 0) {
        session->transmition_timeout = J1939_TP_TO_T4;
        session->state = J1939_TP_STATE_WAIT_CTS;
        return 0;
    }

    memset(&tp_dt, 0xFF, sizeof(j1939_tp_dt));

    msg_start  = (session->pkt_next - 1) * J1939_MULTIPACKET_DATA_SZ;
    msg_sz_min = U8_MIN(J1939_MULTIPACKET_DATA_SZ, (session->msg_sz - msg_start));

    /* copy transmition data into payload */
    memcpy(tp_dt.payload, &session->buffer[msg_start], msg_sz_min);
    tp_dt.seq_num = session->pkt_next;

    level = j1939_bsp_lock();

    if (__send_TPDT(self_addr, session->dst_addr, &tp_dt) < 0) {
        if (session->mode == J1939_TP_MODE_RTS)
            __send_Conn_Abort(self_addr, session->dst_addr, session->PGN, J1939_CONN_ABORT_REASON_NO_RESOURCES);
        __close_tp_session(tp_mgr_ctx, session->id);
        j1939_bsp_unlock(level);
        return 0;
    }

    if (--session->pkt_max == 0) {
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

    j1939_bsp_unlock(level);

    return 1;
}


/**
 * @brief
 *
 * @param tp_mgr_ctx
 * @param time_ms
 */
void j1939_tp_mgr_process(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint32_t t_delta) {
    register int i;
    int level;
    uint8_t CA_addr = j1939_get_address();

    if (tp_mgr_ctx->reset) {
        level = j1939_bsp_lock();
        j1939_tp_mgr_init(tp_mgr_ctx);
        j1939_bsp_unlock(level);
        return;
    }

    if (CA_addr == J1939_NULL_ADDRESS)
        return;

    for (i = 0; i < J1939_TP_SESSIONS_NUM; ++i) {
        j1939_tp_session *const session = &tp_mgr_ctx->sessions[i];

        if (__tp_mgr_process_timeout_check(CA_addr, tp_mgr_ctx, session, t_delta))
            continue;

        (void)__tp_mgr_process_transmition(CA_addr, tp_mgr_ctx, session);
    }
}


/**
 * @brief
 *
 * @return
 */
int j1939_tp_mgr_open_tx_session(j1939_tp_mgr_ctx *const tp_mgr_ctx, uint32_t PGN, uint8_t dst_addr, uint16_t msg_sz, const void *const payload) {
    int sid;
    j1939_tp_session *session;
    j1939_tp_cm_control tp_cm_control;
    int level;
    unsigned pkt_num;

    if (dst_addr == J1939_NULL_ADDRESS || msg_sz > J1939_MAX_DATA_SZ)
        return -EINVAL;

    /* number of packets should be 2 or more */
    pkt_num = (msg_sz - 1) / J1939_MULTIPACKET_DATA_SZ + 1;
    if (pkt_num < 2)
        return -EINVAL;

    if (dst_addr == J1939_GLOBAL_ADDRESS) {
        tp_cm_control = __new_tp_cm_BAM(msg_sz, pkt_num, PGN);
    } else {
        tp_cm_control = __new_tp_cm_RTS(msg_sz,
                                        pkt_num,
                                        U8_MIN(pkt_num, J1939_TP_MGR_MAX_PACKETS_PER_CTS),
                                        PGN);
    }

    level = j1939_bsp_lock();
    sid = __open_tx_session(tp_mgr_ctx, dst_addr, &tp_cm_control);
    j1939_bsp_unlock(level);

    if (sid < 0)
        return -ENOMEM;

    session = &tp_mgr_ctx->sessions[sid];

    /* copy data to dedicated buffer */
    memcpy(session->buffer, payload, msg_sz * sizeof(uint8_t));

    level = j1939_bsp_lock();

    if (__send_TPCM(session->src_addr, session->dst_addr, &tp_cm_control) < 0) {
        __close_tp_session(tp_mgr_ctx, sid);
        j1939_bsp_unlock(level);
        return -EIO;
    }

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
int j1939_tp_mgr_close_session(j1939_tp_mgr_ctx *const tp_mgr_ctx, int sid) {
    int level = j1939_bsp_lock();
    int status = __close_tp_session(tp_mgr_ctx, sid);
    j1939_bsp_unlock(level);
    return status;
}
