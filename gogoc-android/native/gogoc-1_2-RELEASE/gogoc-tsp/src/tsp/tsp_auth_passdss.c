/*
-----------------------------------------------------------------------------
 $Id: tsp_auth_passdss.c,v 1.2 2010/03/07 20:13:23 carl Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"
#include "gogoc_status.h"

#ifndef NO_OPENSSL

#include <openssl/bio.h>
#include <openssl/sha.h>
#include <openssl/dh.h>
#include <openssl/dsa.h>
#include <openssl/hmac.h>

#include "tsp_auth_passdss.h"
#include "buffer.h"
#include "bufaux.h"
#include "log.h"
#include "hex_strings.h"
#include "base64.h"
#include "cli.h"        // ask()
#include "tsp_redirect.h"
#include "tsp_client.h"

#define TSP_AUTH_PASSDSS_STRING "PASSDSS-3DES-1"
#define TSP_AUTH_PASSDSS_BUFFERSIZE 4096
#define TSPC_DSA_KEYFILE "gogockeys.pub"

#if defined(WIN32) && !defined(WINCE)
extern BOOL IsService;    // Declared in winpc/tsp_local.c
#endif


static
int freadline(char *buf, int max_len, FILE *fp)
{
  int pos;
  char c;

  pos = 0;
  while ( ((c=(char)fgetc(fp)) != EOF)  &&  pos<max_len) {
    if ( c == '\n' ) {
      buf[pos++] = c;
      buf[pos] = '\n';
      return 1;
    } else {
      buf[pos++] = c;
    }
  }

  return EOF;
}


/* is_dsakey_in_keyfile
 *
 * @returns 0 if error
 * @returns 1 if not in keyfile
 * @returns 2 if in keyfile and matching
 * @returns 3 if in keyfile and not matching
*/

static
int
is_dsakey_in_keyfile(DSA *dsa_1, char *host, char *filename)
{
  FILE *fp = NULL;
  char *line_buf = NULL;
  char *str = NULL;
  char *str_base64 = NULL;
  Buffer keyfile_buf;
  BIGNUM *bn_p = NULL;
  BIGNUM *bn_q = NULL;
  BIGNUM *bn_g = NULL;
  BIGNUM *bn_pub_key = NULL;
  int ret = 1;
  size_t str_size = 0;
  int i;


  if (!dsa_1 || !host || !filename)
    return 0;

  memset(&keyfile_buf, 0, sizeof(keyfile_buf));

  /* Open the keyfile */

  if ( (fp = fopen(filename, "r")) == NULL) {
    /* no key file */
    ret = 1;
    goto is_dsakey_in_keyfile_cleanup;
  }

  /* Get memory for a line buffer */

  if ( (line_buf = pal_malloc(sizeof(char) * 4096)) == NULL) {
    ret = 0;
    goto is_dsakey_in_keyfile_cleanup;
  }

  while (freadline(line_buf, 4095, fp) != EOF) {
    int len;

    str_base64 = strtok(line_buf, " ");

    if (strcmp(host, str_base64) != 0)
      continue;

    /* We got a match on the hostname
     * Now we assemble a DSA object
     * with what we have in the buffer
     */

    str_base64 = strtok(NULL, " ");

    if (strcmp("ssh-dss", str_base64) != 0)
      continue;

    /* We got a match on the key type as
       well, going on
    */

    str_base64 = strtok(NULL, " ");

    /*
     * Get the base64 data
     * and discard ssh-dss first
     */

    str_size = pal_strlen(str_base64) * sizeof(char);

    if ( (str = pal_malloc(str_size)) == NULL) {
      ret = 0;
      goto is_dsakey_in_keyfile_cleanup;
    }

    if ( (len = base64decode(str, str_base64)) < 1) {
      ret = 0;
      goto is_dsakey_in_keyfile_cleanup;
    }

    /* Fill a buffer structure with this data
     */

    buffer_init(&keyfile_buf);
    if (keyfile_buf.buf == NULL) {
      ret = 0;
      goto is_dsakey_in_keyfile_cleanup;
    }

    buffer_append(&keyfile_buf, str, str_size);

    /* Now get some BN filled with the data
     * from the buffer and compare
     * those with what we got in the incoming
     * dsa object.
     */

    bn_p = BN_new();
    bn_q = BN_new();
    bn_g = BN_new();
    bn_pub_key = BN_new();

    if (bn_p == NULL ||
  bn_q == NULL ||
  bn_g == NULL ||
  bn_pub_key == NULL) {
      ret = 0;
      goto is_dsakey_in_keyfile_cleanup;
    }

    /* String is skipped XXX */
    buffer_get_string(&keyfile_buf, (unsigned int*)&i);  // Bogus i returns the string length, not used
    buffer_get_bignum(&keyfile_buf, bn_p);
    buffer_get_bignum(&keyfile_buf, bn_q);
    buffer_get_bignum(&keyfile_buf, bn_g);
    buffer_get_bignum(&keyfile_buf, bn_pub_key);

    if ( !BN_cmp(dsa_1->p, bn_p) &&
   !BN_cmp(dsa_1->q, bn_q) &&
   !BN_cmp(dsa_1->g, bn_g) &&
   !BN_cmp(dsa_1->pub_key, bn_pub_key) )
      ret = 2;
    else ret = 3;
  }

is_dsakey_in_keyfile_cleanup:

    if (str)
      pal_free(str);
    if (line_buf)
      pal_free(line_buf);
    if (fp)
      fclose(fp);
    if (bn_p)
      BN_clear_free(bn_p);
    if (bn_q)
      BN_clear_free(bn_q);
    if (bn_g)
      BN_clear_free(bn_g);
    if (bn_pub_key)
      BN_clear_free(bn_pub_key);
    if (keyfile_buf.buf)
      buffer_free(&keyfile_buf);

  return ret;
}


/** Add a DSA key to the tspc key file
 *
 * @param dsa        the DSA param pointer filled with our key info
 * @param host       the hostname of the corresponding broker
 * @param filename   the keyfile to use
 *
 * @return  0 if error
 *          1 if ok
 *
 */
int
add_dsakey_to_keyfile(DSA *dsa, char *host, char *filename, tBoolean autoaccept)
{

  FILE *fp = NULL;
  Buffer buf;
  char *str = NULL;
  int ret = 0;

  switch (is_dsakey_in_keyfile(dsa, host, filename)) {

  case 0:
    Display(LOG_LEVEL_3, ELInfo, TSP_AUTH_PASSDSS_STRING, GOGO_STR_ERR_IN_KEY_VERIF);
    Display(LOG_LEVEL_3, ELWarning, TSP_AUTH_PASSDSS_STRING, GOGO_STR_SERVER_KEY_REJECTED);
    break;
  case 1: /* not in, we add and continue */
#if defined(WIN32) && !defined(WINCE)
// When running as a service we can't ask user
// permission. Compromise and accept the key auto
//
    if (!IsService && !autoaccept)
    {
#else
    if (!autoaccept)
    {
#endif
      if (!ask(GOGO_STR_UNKNOWN_HOST_ADD_KEY, host))
      {
        Display(LOG_LEVEL_3, ELWarning, TSP_AUTH_PASSDSS_STRING, GOGO_STR_SERVER_KEY_REJECTED_USER);
        break;
      }
    }
    else
  Display(LOG_LEVEL_1, ELWarning, TSP_AUTH_PASSDSS_STRING, GOGO_STR_WARN_SERVER_KEY_AUTO_ADDED);

    Display(LOG_LEVEL_2, ELInfo, TSP_AUTH_PASSDSS_STRING, GOGO_STR_SERVER_KEY_ACCEPTED_ADDED);

    buffer_init(&buf);
    if (buf.buf == NULL)
      break;
    buffer_put_cstring(&buf, "ssh-dss");
    buffer_put_bignum(&buf, dsa->p);
    buffer_put_bignum(&buf, dsa->q);
    buffer_put_bignum(&buf, dsa->g);
    buffer_put_bignum(&buf, dsa->pub_key);

    if ( (str = pal_malloc(2 * buffer_len(&buf))) == NULL)
      break;

    if ( (base64encode(str, buffer_ptr(&buf), (int) buffer_len(&buf))) < 1)
      break;

    fp = fopen(filename, "a");
    if (fp) {
      fprintf(fp, "%s ssh-dss %s\n", host, str);
      fclose(fp);
      ret = 1;
    }
    buffer_free(&buf);
    pal_free(str);
    break;
  case 2: /* in and matching correctly, hurray */
    Display(LOG_LEVEL_2, ELInfo, TSP_AUTH_PASSDSS_STRING, GOGO_STR_MATCHING_KEY_FOUND_USED);
    ret = 1;
    break;
  case 3: /* in and NOT matching correctly */
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, GOGO_STR_WARN_STORED_LOCAL_KEY_NO_MATCH, filename, host);
  Display(LOG_LEVEL_3, ELWarning, TSP_AUTH_PASSDSS_STRING, GOGO_STR_SERVER_KEY_REJECTED);
    ret = 0;
    break;
  }

  return ret;
}


/**
 * Authenticate to the Migration Broker using PASSDSS-3DES-1
 *
 * Buf_H will contain the data used to validate the server
 * signature. The data is a concatenation of the following parameters,
 * in that order:
 * azname,authname,DH_public_key,pklength,"ssh-dss",p,q,g,z,Y,ssecmask,sbuflen,dh_K
 *
 * @param socket
 * @param user
 * @param passwd
 * @param host
 * @param nt
 *
 * @return
 *
 * @todo DH public key validation  (RFC2631, 2.1.5)
 * @todo Local storage for server public keys
 *
 */
gogoc_status AuthPASSDSS_3DES_1(pal_socket_t socket, net_tools_t *nt, tConf *conf, tBrokerList **broker_list)
{
  DH   *dh = NULL;        /**< client DH key used to exchange key with server */
  DSA  *dsa = NULL; /**< Remote server DSA key public information */
  DSA_SIG *sig = NULL;    /**< DSA signature */
  char authenticate[] = "AUTHENTICATE PASSDSS-3DES-1\r\n";
  char *BufferIn  = NULL;
  char *BufferOut = NULL;
  char *BufferPtr = NULL;
  Buffer BufH;    /**< Buffer to hold data used for signature. */
  Buffer BufSpace;  /**< Space to hold data before/after base64 conversion */
  Buffer *Buf_H = &BufH;
  Buffer *Buf_Space = &BufSpace;
  BIO  *bio_rw = NULL;    /**< Memory buffer bio */
  BIO  *b64= NULL;    /**< Base64 bio */
  BIO  *cipher = NULL;    /**< Symmetric crypto bio */
  BIGNUM *server_pubkey = NULL; /**< received server public DH key */
  BIGNUM *dh_K = NULL;          /**< DH computed shared secret */
  u_char hash[20];  /**< SHA1 hash */
  u_char enc_key[24]; /**< encryption key (3des) */
  u_char enc_iv[8]; /**< initialization vector (3des) */
  u_char int_key[20]; /**< cs integrity key */
  u_char tmphash[40]; /**< temporary hash storage */
  u_char hmac[EVP_MAX_MD_SIZE]; /**< HMAC for integrity of sent data (step L) */
  int  pklength = 0;  /**< length of SSH-style DSA server public key */
  int ssecmask = 0; /**< SASL security layers offered */
  int sbuflen = 0;  /**< maximum server security layer block size */
  char *s = NULL;
  u_char num[3];    /**< Array to manupulate 3 octet number (sbuflen)  */
  /* Temporary variables */
  int  buflen, readlen, keysize, siglength;
  gogoc_status status = STATUS_SUCCESS_INIT;
  sint32_t tsp_status;

/* From draft-newman-sasl-passdss-01.  "This group was taken from the
 * ISAKMP/Oakley specification, and was originally generated by
 * Richard Schroeppel at the University of Arizona.  Properties of
 * this prime are described in [Orm96]"
 */

        /* RFC2409, DH group 2 (second Oakley group) */
  static char *dh_group2=
      "FFFFFFFF" "FFFFFFFF" "C90FDAA2" "2168C234" "C4C6628B" "80DC1CD1"
      "29024E08" "8A67CC74" "020BBEA6" "3B139B22" "514A0879" "8E3404DD"
      "EF9519B3" "CD3A431B" "302B0A6D" "F25F1437" "4FE1356D" "6D51C245"
      "E485B576" "625E7EC6" "F44C42E9" "A637ED6B" "0BFF5CB6" "F406B7ED"
      "EE386BFB" "5A899FA5" "AE9F2411" "7C4B1FE6" "49286651" "ECE65381"
      "FFFFFFFF" "FFFFFFFF";
  static unsigned char dh_g[]={
    0x02,
  };


  /* Initialize Diffie Hellman variables */
  if ((dh = DH_new()) == NULL || (server_pubkey = BN_new()) == NULL)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_GEN_MALLOC_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    goto error;
  }
  /* Convert dh_group2 and dh_g to BIGNUM type */
  BN_hex2bn(&dh->p, dh_group2);
  dh->g = BN_bin2bn(dh_g,sizeof(dh_g),NULL);
  if ((dh->p == NULL) || (dh->g == NULL))
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, GOGO_STR_INITIALIZATION_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
    goto error;
  }
  if ((dh_K = BN_new()) == NULL)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_GEN_MALLOC_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    goto error;
  }

  /* Reserve storage for DSA key */
  if ((dsa = DSA_new()) == NULL || (dsa->p = BN_new()) == NULL ||
      (dsa->q = BN_new()) == NULL ||  (dsa->g = BN_new()) == NULL ||
      (dsa->pub_key = BN_new()) == NULL || (dsa->priv_key = BN_new()) == NULL)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_GEN_MALLOC_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    goto error;
  }

  /* Allocate memory for DSA signature */
  if ((sig = DSA_SIG_new()) == NULL)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_GEN_MALLOC_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    goto error;
  }
  /* Initialize data buffers */
  BufferIn  = calloc(1, TSP_AUTH_PASSDSS_BUFFERSIZE);
  BufferOut = calloc(1, TSP_AUTH_PASSDSS_BUFFERSIZE);

  if ((BufferIn == NULL) || (BufferOut == NULL))
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_GEN_MALLOC_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    goto error;
  }
  buffer_init(Buf_Space);
  buffer_init(Buf_H);
  if (Buf_Space->buf == NULL || Buf_H->buf == NULL)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_GEN_MALLOC_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    goto error;
  }
  /* Create a read/write memory BIO. Memory is segment is
   * created and resized as needed. When BIO is destroyed, the
   * memory is freed. */
  bio_rw = BIO_new(BIO_s_mem());
  /* Create a base64 BIO filter */
  b64 = BIO_new(BIO_f_base64());
  if ((bio_rw == NULL) || (b64 == NULL))
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_GEN_MALLOC_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    goto error;
  }

  /*
    Compute the Diffie-Hellman public value "X" as follows.  If
    X has a value of 0, repeat.

         x
    X = g  mod n

    where g = dh_g = 2
          n = dh_group2
    x = DH secret key
    X = DH public key
  */
  if (DH_generate_key(dh) == 0)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, GOGO_STR_DH_GEN_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
    goto error;
  }

  /* Validate DH public key (RFC2631, 2.1.5) */

  /* Send  message with SASL mechanism identifier */
  if ( nt->netsend(socket, authenticate, sizeof(authenticate)) == -1 )
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_NET_FAIL_W_SOCKET);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_SOCKET_IO);
    goto error;
  }

  /* First PASSDSS  message from client to server:
     string azname       ; the user name to login as, may be empty if
                           same as authentication name
     string authname     ; the authentication name
     mpint  X            ; Diffie-Hellman parameter X
  */
  /* azname is empty. Just insert a string length zero */
  buffer_put_int(Buf_Space, 0);
  /* authname */
  buffer_put_cstring(Buf_Space, conf->userid);
  /* DH public key */
  buffer_put_bignum(Buf_Space, dh->pub_key);

  /* At this point, save the buffer into Buf_H. Used later for
   * signature verification. */
  buffer_append(Buf_H, buffer_ptr(Buf_Space), buffer_len(Buf_Space));

  /* Push base64 filter */
  BIO_push(b64, bio_rw);
  /* no newline */
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  /* Write Buffer content into bio_rw. Buffer will be base64
   * encoded. */
  BIO_write(b64, buffer_ptr(Buf_Space), (int) buffer_len(Buf_Space));
  BIO_flush(b64);
  /* Get pointer to the result */
  buflen = BIO_get_mem_data(bio_rw, &BufferPtr);

  // Send data to server, save response in BufferIn.
  if((readlen = nt->netsendrecv(socket,
               BufferPtr, buflen,
               BufferIn, TSP_AUTH_PASSDSS_BUFFERSIZE)) == -1)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_NET_FAIL_RW_SOCKET);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_SOCKET_IO);
    goto error;
  }
  /* remove base64 filter */
  BIO_pop(bio_rw);
  buffer_clear(Buf_Space);
  buflen = 0;

  /* Decode response (base64) and extract server response
   *
   * The response format is as follows:

       uint32   pklength   ; length of SSH-style DSA server public key
           (number of bytes up to y, inclusively)
         string "ssh-dss"  ; constant string "ssh-dss" (lower case)
         mpint  p          ; DSA public key parameters
         mpint  q
         mpint  g
         mpint  z            (y in draft)
       mpint    Y          ; Diffie-Hellman parameter Y
       OCTET    ssecmask   ; SASL security layers offered
       3 OCTET  sbuflen    ; maximum server security layer block size
       uint32   siglength  ; length of SSH-style dss signature
           (number of bytes up to s inclusively)
         string "ssh-dss"  ; constant string "ssh-dss" (lower case)
         mpint  r          ; DSA signature parameters
         mpint  s

   */

  buflen = base64decode(BufferOut, BufferIn);

  buffer_append(Buf_Space, BufferOut, buflen);
  /* Get pklength */
  pklength = buffer_get_int(Buf_Space);
  /* Assuming that
   * p, g, and y are 512 bits,
   * q is 160 bits,
   * "ssh-dss" is 7 bytes
   * pklength should be at least 240 bytes.
   */
  if (pklength < 240)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, GOGO_STR_RCVD_DATA_INVALID);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
    goto error;
  }

  /* Make a copy of (pklength|"ssh-dss"|p|q|g|z) in Buf_H */
  /* Add pklength */
  buffer_put_int(Buf_H, pklength);
  /* Add "ssh-dss"|p|q|g|z */
  buffer_append(Buf_H, buffer_ptr(Buf_Space), pklength);

  /* Get "ssh-dss" string */
  s = buffer_get_string(Buf_Space, (unsigned int*)&buflen);
  pal_free(s); s = NULL;
  /* Get p */
  buffer_get_bignum(Buf_Space, dsa->p);
  /* Get q */
  buffer_get_bignum(Buf_Space, dsa->q);
  /* Get g */
  buffer_get_bignum(Buf_Space, dsa->g);
  /* Get z (pub_key) */
  buffer_get_bignum(Buf_Space, dsa->pub_key);
  /* Get DH public key */
  buffer_get_bignum(Buf_Space, server_pubkey);
  /* Copy in Buf_H for signature verification later */
  buffer_put_bignum(Buf_H, server_pubkey);

  /* Buffer now points at ssecmask (1 octet), followed by
   * sbuflen (3 octets). Make a copy of these 4 octets in Buf_H
   * now, then extract these values. */
  buffer_append(Buf_H, buffer_ptr(Buf_Space), 4);

  /* Get ssecmask */
  ssecmask = buffer_get_octet(Buf_Space);
  /* Get sbuflen
   * Big endian binary unsigned integer */
  buffer_get(Buf_Space, (char *)num, 3);
  sbuflen =  (((u_long)(u_char)(num)[0] << 16) |
        ((u_long)(u_char)(num)[1] << 8) |
        ((u_long)(u_char)(num)[2]));

  /* DSS signature */
  /* Get siglength */
  siglength = buffer_get_int(Buf_Space);
  /* r and s are 20 bytes each, encoded as mpint (2*24)
   * "ssh-dss" is 7 bytes + int32 siglength should be >= 59
   * octets (mpint may have leading zero byte)
   */
  if (siglength < 59)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, GOGO_STR_RCVD_DATA_INVALID);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
    goto error;
  }
  /* Get "ssh-dss" string */
  s = buffer_get_string(Buf_Space, (unsigned int*)&buflen);
  pal_free(s); s = NULL;
  /* Get DSA signature r and s*/
  if ((sig->r= BN_new()) == NULL || (sig->s = BN_new()) == NULL)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_GEN_MALLOC_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    goto error;
  }
  /* Get r */
  buffer_get_bignum(Buf_Space, sig->r);
  /* Get s */
  buffer_get_bignum(Buf_Space, sig->s);

  /* Validate server DH public key  (RFC2631, 2.1.5) */

  {
    if( !add_dsakey_to_keyfile(dsa, conf->server, TSPC_DSA_KEYFILE, conf->no_questions) )
    {
      Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, GOGO_STR_KEY_VERIF_ERROR);
      status = make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
      goto error;
    }
  }

  /* Verify that DSA public key belongs to server */

  /* Compute DH shared secret */
  if ((s = calloc(1, DH_size(dh))) == NULL)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_GEN_MALLOC_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_MEMORY_STARVATION);
    goto error;
  }
  if( (keysize = DH_compute_key((unsigned char*)s, server_pubkey, dh)) < 0 )
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, GOGO_STR_DH_SHARED_COMPUTE_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
    goto error;
  }
  BN_bin2bn((const unsigned char*)s, keysize, dh_K);
  memset(s, 0, keysize);
  pal_free(s);
  s = NULL;
  Display(LOG_LEVEL_3, ELDebug, TSP_AUTH_PASSDSS_STRING,
    GOGO_STR_DH_SHARED_KEY, BN_bn2hex(dh_K));

  /* Append dh_K in to complete the buffer. Use Buffer to hold
   * result to keep Bf_H intact, since to will be used (without
   * dh_K) to compute HMAC for packet integrity. */
  buffer_clear(Buf_Space);
  buffer_append(Buf_Space, buffer_ptr(Buf_H), buffer_len(Buf_H));
  buffer_put_bignum(Buf_Space, dh_K);

  /* Compute SHA1 hash of Buffer */
  SHA1(buffer_ptr(Buf_Space), buffer_len(Buf_Space), hash);

  /* Debug information available at level 4 */
 {
   BIGNUM *h;
   h = BN_bin2bn(hash, 20, NULL);
   Display(LOG_LEVEL_3, ELDebug, TSP_AUTH_PASSDSS_STRING,
     GOGO_STR_SIGNED_HASH, BN_bn2hex(h));
   BN_free(h);
 }
   Display(LOG_LEVEL_3, ELDebug, TSP_AUTH_PASSDSS_STRING,
     GOGO_STR_DSA_SIGN_R, BN_bn2hex(sig->r));
   Display(LOG_LEVEL_3, ELDebug, TSP_AUTH_PASSDSS_STRING,
     GOGO_STR_DSA_SIGN_S, BN_bn2hex(sig->s));

  // Verify that the DSS signature is a signature of hash.
  switch( DSA_do_verify(hash, sizeof(hash), sig, dsa) )
  {
  case 0:
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, GOGO_STR_BAD_SIG_FROM_SERVER);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
    goto error;
    break; /* NOTREACHED */

  case 1:  /* correct signature */
    break;

  default: /* -1 on error */
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, GOGO_STR_SIG_VERIF_ERROR);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
    goto error;
    break; /* NOTREACHED */
  }

  /* Step I: Compute 3DES key and iv */
  /*
    cs-encryption-iv    = SHA1( K || "A" || H )
    sc-encryption-iv    = SHA1( K || "B" || H )
    cs-encryption-key-1 = SHA1( K || "C" || H )
    cs-encryption-key-2 = SHA1( K || cs-encryption-key-1 )
    cs-encryption-key   = cs-encryption-key-1 || cs-encryption-key-2
    sc-encryption-key-1 = SHA1( K || "D" || H )
    sc-encryption-key-2 = SHA1( K || sc-encryption-key-1 )
    sc-encryption-key   = sc-encryption-key-1 || sc-encryption-key-2
    cs-integrity-key    = SHA1( K || "E" || H )
    sc-integrity-key    = SHA1( K || "F" || H )

    K is dh_k in mpint format (string)
    H is hash
  */

  /* Since we won't support SASL security layers, we need to
   * compute the following only:
   * cs-encryption-iv
   * cs-encryption-key
   * cs-integrity-key
   */
  buffer_clear(Buf_Space);
  buffer_put_bignum(Buf_Space, dh_K);
  buffer_put_octet(Buf_Space,'A');
  buffer_append(Buf_Space, hash, 20);
  SHA1(buffer_ptr(Buf_Space), buffer_len(Buf_Space), tmphash);
  /* Use first 8 octets as iv */
  memcpy(enc_iv, tmphash, 8);

  buffer_clear(Buf_Space);
  buffer_put_bignum(Buf_Space, dh_K);
  buffer_put_octet(Buf_Space,'E');
  buffer_append(Buf_Space, hash, 20);
  SHA1(buffer_ptr(Buf_Space), buffer_len(Buf_Space), int_key);

  buffer_clear(Buf_Space);
  buffer_put_bignum(Buf_Space, dh_K);
  buffer_put_octet(Buf_Space,'C');
  buffer_append(Buf_Space, hash, 20);
  SHA1(buffer_ptr(Buf_Space), buffer_len(Buf_Space), tmphash);
  buffer_clear(Buf_Space);
  buffer_put_bignum(Buf_Space, dh_K);
  buffer_append(Buf_Space, tmphash, 20);
  SHA1(buffer_ptr(Buf_Space), buffer_len(Buf_Space), tmphash+20);
  /* Use first 24 octets as key */
  memcpy(enc_key, tmphash, 24);
 {
   BIGNUM *enc, *i, *iv;
   enc = BN_bin2bn(enc_key, 24, NULL);
   iv = BN_bin2bn(enc_iv, 8, NULL);
   i = BN_bin2bn(int_key, 20, NULL);

   Display(LOG_LEVEL_3, ELDebug, TSP_AUTH_PASSDSS_STRING,
     GOGO_STR_PASSDS_ENC_KEY, BN_bn2hex(enc));
   Display(LOG_LEVEL_3, ELDebug, TSP_AUTH_PASSDSS_STRING,
     GOGO_STR_PASSDS_IV, BN_bn2hex(iv));
   Display(LOG_LEVEL_3, ELDebug, TSP_AUTH_PASSDSS_STRING,
     GOGO_STR_PASSDS_INTEG_KEY, BN_bn2hex(i));
   BN_free(enc);
   BN_free(i);
   BN_free(iv);
 }
  /*
    (J) Create a buffer beginning with a bit mask for the
    selected security layer (it MUST be one offered from server)
    followed by three octets representing the maximum
    cipher-text buffer size (at least 32) the client can accept
    in network byte order.  This is followed by a string
    containing the passphrase.
  */
  buffer_clear(Buf_Space);
  buffer_put_octet(Buf_Space, ssecmask);
  buffer_put_octet(Buf_Space, 0);
  buffer_put_octet(Buf_Space, 0);
  buffer_put_octet(Buf_Space, 0); /**< @bug must be at least 32 */
  buffer_put_cstring(Buf_Space, conf->passwd);

  /*
    (K) Create a buffer containing items (1) through (7)
    immediately followed by the first four octets of (J).
  */
  buffer_append(Buf_H, buffer_ptr(Buf_Space), 4);

  /*
    (L) Compute HMAC-SHA-1 with (K) as the data and the
    cs-integrity- key from step (I) as the key.  This produces a
    20 octet result.
  */
  HMAC(EVP_sha1(), int_key, sizeof(int_key),
       buffer_ptr(Buf_H), buffer_len(Buf_H), hmac, (unsigned int*)&keysize);
  /*
    (M) Create a buffer containing (J) followed by (L) followed
    by an arbitrary number of zero octets as necessary to reach
    the block size of DES and conceal the passphrase length from
    an eavesdropper.
  */
  buffer_append(Buf_Space, hmac, keysize);

  /*
    (N) Apply the triple-DES algorithm to (M) with the first 8
    octets of cs-encryption-iv from step (I) as the
    initialization vector and the first 24 octets of
    cs-encryption-key as the key.
  */
  /*
    Padding is automatically done. From OpenSSL EVP_EncryptInit(3):
    EVP_CIPHER_CTX_set_padding() enables or disables padding. By default
    encryption operations are padded using standard block padding and the
    padding is checked and removed when decrypting.
  */

  /*
    Create BIO filter to encrypt using 3des + convert to
    base64. Result is written in memory BIO.
  */
  /* Erase BIO and buffer memory */
  BIO_reset(bio_rw);
  memset(BufferOut, 0, TSP_AUTH_PASSDSS_BUFFERSIZE);
  memset(BufferIn, 0, TSP_AUTH_PASSDSS_BUFFERSIZE);
  buflen = 0;

  /* Create cipher BIO */
  cipher = BIO_new(BIO_f_cipher());
  BIO_set_cipher(cipher, EVP_des_ede3_cbc(), enc_key, enc_iv, 1);
  /* Assemble filters as cipher->b64->bio_rw */
  BIO_push(cipher, b64);
  BIO_push(b64, bio_rw);

  /* Write Buffer content into bio_rw */
  BIO_write(cipher, buffer_ptr(Buf_Space), (int) buffer_len(Buf_Space));
  BIO_flush(cipher);
  /* Get pointer to the result. */
  buflen = BIO_get_mem_data(bio_rw, &BufferPtr);

  /* wipe encryption material */
  memset(enc_key, 0, sizeof(enc_key));
  memset(enc_iv, 0, sizeof(enc_iv));

  /* Send data to server, save response in BufferIn */
  if( (readlen = nt->netsendrecv(socket, BufferPtr, buflen,
       BufferIn, TSP_AUTH_PASSDSS_BUFFERSIZE)) == -1)
  {
    Display(LOG_LEVEL_1, ELError, TSP_AUTH_PASSDSS_STRING, STR_NET_FAIL_RW_SOCKET);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_SOCKET_IO);
    goto error;
  }
  tsp_status = tspGetStatusCode(BufferIn);

  // Check if the reply status indicated a broker redirection.
  if( tspIsRedirectStatus(tsp_status) )
  {
    if( tspHandleRedirect(BufferIn, conf, broker_list) == TSP_REDIRECT_OK )
    {
      status = make_status(CTX_TSPAUTHENTICATION, EVNT_BROKER_REDIRECTION);
    }
    else
    {
      // Redirect error.
      status = make_status(CTX_TSPAUTHENTICATION, ERR_BROKER_REDIRECTION);
    }
    goto error;
  }

  // Check if authentication was successful.
  switch( tsp_status )
  {
  case TSP_PROTOCOL_SUCCESS:
    break;

  case TSP_PROTOCOL_AUTH_FAILED:
    Display(LOG_LEVEL_1, ELError, "AuthPASSDSS_3DES_1", STR_TSP_AUTH_FAILED_USER, conf->userid);
    status = make_status(CTX_TSPAUTHENTICATION, ERR_AUTHENTICATION_FAILURE);
    goto error;

  default:
    Display(LOG_LEVEL_1, ELError, "AuthPASSDSS_3DES_1", STR_TSP_UNKNOWN_ERR_AUTH_FAILED, tspGetTspStatusStr(tsp_status));
    status = make_status(CTX_TSPAUTHENTICATION, ERR_TSP_GENERIC_ERROR);
    goto error;
  }

  status = STATUS_SUCCESS_INIT;

 error:

  /* Free storage for DSA key */
  if (dsa != NULL) DSA_free(dsa); /* Also frees BIGNUMs inside struct */
  /* DSA signature */
  if (sig != NULL) DSA_SIG_free(sig);
  /* Free Diffie Hellman variables */
  if (dh != NULL) DH_free(dh); /* Also frees BIGNUMs inside struct */
  if (server_pubkey != NULL) BN_free(server_pubkey);
  if (dh_K != NULL) BN_free(dh_K);
  /* Buffers */
  if (Buf_Space->buf != NULL) buffer_free(Buf_Space);
  if (Buf_H->buf != NULL)  buffer_free(Buf_H);
  /* malloc'ed space*/
  if (BufferIn != NULL) pal_free(BufferIn);
  if (BufferOut != NULL) pal_free(BufferOut);
  /* BIOs */
  if (cipher != NULL) BIO_vfree(cipher);
  if (b64 != NULL) BIO_vfree(b64);
  if (bio_rw != NULL) BIO_vfree(bio_rw);
  /* strings buffers */
  if (s != NULL) pal_free(s);

  return status;
}

#endif
