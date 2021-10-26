/**
 * @file j1939_std_pgn.h
 *
 * @brief
 */


#ifndef J1939_STD_PGN_H_
#define J1939_STD_PGN_H_

#define J1939_STD_PGN_ACKM              59392U
#define J1939_STD_PGN_ACKM_DLC          8

#define J1939_STD_PGN_RQST              59904U
#define J1939_STD_PGN_RQST_DLC          3

#define J1939_STD_PGN_RQST2             51456U
#define J1939_STD_PGN_RQST2_DLC         8

#define J1939_STD_PGN_TPDT              60160U
#define J1939_STD_PGN_TPDT_DLC          8

#define J1939_STD_PGN_TPCM              60416U
#define J1939_STD_PGN_TPCM_DLC          8

#define J1939_STD_PGN_ACLM              60928U
#define J1939_STD_PGN_ACLM_DLC          8

#define J1939_STD_PGN_PROPA             61184U
// no DLC

#define J1939_STD_PGN_PROPA2            126720U
// no DLC

#define J1939_STD_PGN_PROPB(x)          (65280U + (x))
// no DLC

#define J1939_STD_PGN_TRANSFER          51712U
// no DLC

#endif /* J1939_STD_PGN_H_ */
