/***************************************************************************
** All right reserved:  Newland Copyright (c) 2016-2026  
** File name:  lsecurity.h
** File indentifier: 
** Brief:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-26
** Modify record: 
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#ifndef _LSECRITY_H_
#define _LSECRITY_H_

//#include "napi_crypto.h"
#include "emvl3.h"
/** @addtogroup Security
* @{
*/

#define DESMODE_DES  '0'		/**< DES MODE*/
#define DESMODE_3DES '1'		/**< 3DES MODE*/

#define DESMODE_UNDES  '2'		/**< UNDES MODE*/
#define DESMODE_3UNDES '3'		/**< 3UNDES MODE*/

#define FORCE_EXIT_PINMODE do{ \
		NAPI_SecVPPSetEvent(NAPI_SEC_VPP_KEY_ESC); \
		NAPI_SecVPPGetEvent(&nEvent, (uchar *)sPin, &nOutPinLen, szKsn, &nKsnLen); \
		NAPI_SecVPPSetEvent(NAPI_SEC_VPP_KEY_ESC); \
		NAPI_SecVPPGetEvent(&nEvent, (uchar *)sPin, &nOutPinLen, szKsn, &nKsnLen); \
    }while(0);

#define _FINISH_PIN_INPUT_THREAD(nRet) do{PubDebug("_FINISH_PIN_INPUT_THREAD : nRet = %d", nRet); FORCE_EXIT_PINMODE; return nRet;}while(0)

typedef enum 
{
	SECRITY_KEYSYSTEM_MSKEY=0x00,  			/**<MSKEY*/
	SECRITY_KEYSYSTEM_DUKPT,				/**<DUKPT*/
}EM_KEYSYSTEM;

typedef enum 
{
	SECRITY_MODE_INSIDE=0x00,  				/**<Built-in Security module inside*/
	SECRITY_MODE_PINPAD,					/**<Use pinpad*/
}EM_SECRITY_MODE;

enum EM_KEY_TYPE
{
	KEY_TYPE_PIN=0x00,  			    /**<Pin key*/
	KEY_TYPE_MAC,					    /**<Mac key*/	
	KEY_TYPE_DATA,				        /**<Data key*/
	KEY_TYPE_TMK,						/**<Master key*/
};

enum EM_MAC_TYPE
{
	MAC_TYPE_ECB=0,						/**<ECB (TMK & dukpt)*/
	MAC_TYPE_X99,						/**<X99 (TMK & dukpt)*/
	MAC_TYPE_X919,						/**<X919 (TMK & dukpt)*/
	MAC_TYPE_9606,						/**<Mac 9606 (TMK & dukpt)*/
	MAC_TYPE_AES_X99,					/**<AES x99 (TMK & dukpt)*/
	MAC_TYPE_AES_X9606,					/**<AES x99 (TMK & dukpt)*/
	MAC_TYPE_ECB_RESP,					/**<ECB RES only for dukpt*/
	MAC_TYPE_X99_RESP,					/**<X99 RESP only for dukpt*/
	MAC_TYPE_X919_RESP,					/**<X919 RESP only for dukpt*/
	MAC_TYPE_9606_RESP,					/**<Mac 9606 only for dukpt*/
	MAC_TYPE_AES_X99_RESP, 				/**<AES X99 RESP only for dukpt*/
	MAC_TYPE_AES_X9606_RESP,			/**<AES X9606 RESP only for dukpt*/
};

enum EM_PIN_TYPE
{
   PINTYPE_WITHPAN = 0,       	    /**<With PAN encrypt*/
   PINTYPE_WITHOUTPAN = 1,     		/**<Without PAN encrypt*/
   PINTYPE_PLAIN_STAR = 2,       	/**<No encrypt,Show PIN replaced by '*' */
   PINTYPE_PLAIN = 3,               /**<No encrypt,Show clear PIN*/
};

typedef struct{
	char cAux;             /**<The serial port connected to PINPAD(built-in ignored)*/
	char cTimeout;	       /**<Timeout, used when getting pin block and detecting pinpad*/
	char sReserve[63];     /**<Reserve-*/
}STPINPADPARAM;

/**
* @brief Init security module
* @details Will check if the module is existed
* @param [in] nMode           ---Refer to EM_SECRITY_MODE
* @param [in] pstPinpadParam  ---Pinpad parameters
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubInitSecrity(int nMode, const STPINPADPARAM* pstPinpadParam);

/**
* @brief Clear KEY
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubClearKey(void);

/**
* @brief Intall main key
* @param [in] nIndex  >= 1
* @param [in] psKey   The buffer of main key
* @param [in] psKSN   The buffer of KSN used in DUKPT mode
* @param [in] nKeyLen key length is 8 or 16
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubLoadMainKey(int nIndex, const char *psKey, const char *psKSN, int nKeyLen);

/**
* @brief Set the index of current main key.
* @param [in] nIndex Index of key
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubSetCurrentMainKeyIndex(int nIndex);

/**
* @brief Set the key system type.
* @param [in] nType Type of key system--EM_KEYSYSTEM
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubSetKeySystemType(int nType);

/**
* @brief Install word key(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in] nKeyType      Key type---EM_KEY_TYPE
* @param [in] psKey         Buffer of the working key
* @param [in] nKeyLen       key length 16
* @param [in] psCheckValue  The buffer of key check value (3 bytes) or NULL
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubLoadWorkKey(int nKeyType, const char *psKey, int nKeyLen, const char *psCheckValue);

/**
* @brief Get PIN-Block(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [out] psPin         Pointer to PIN-block buffer
* @param [out] pnPinLen      Pointer to PIN-block-length buffer
* @param [in]  nMode         EM_PIN_TYPE
* @param [in]  pszCardno     Card Number/(NULL)
* @param [in]  pszAmount     Amount(used for display, e.g. buff:000000001234-->display:12.34)
* @param [in]  nMaxLen       Maximum PIN length (4-12)
* @param [in]  nMinLen       Minimun PIN length
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubGetPin(char *psPin, int *pnPinLen, int nMode, const char *pszCardno, const char *pszAmount,
			  int nMaxLen, int nMinLen);

/**
* @brief Calculate MAC(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in]  nMode         Method---EM_MAC_TYPE
* @param [in]  psData        Data that needs to be calculated
* @param [in]  nDataLen      Data length
* @param [out] psMac         MAC
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubCalcMac(int nMode, const char *psData, int nDataLen, char *psMac);

/**
* @brief Increase KSN
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubDukptIncreaseKSN(void);

/**
* @brief Get KSN
* @param [out] psKSN    KSN data---10byte
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubGetDukptKSN(char *psKSN);

/**
* @brief Do TDES encryption with specified 'nKeyType' key(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in]  nLen          Length of data
* @param [in]  psSrc         Source data
* @param [out] psDest        Target data
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubDes3(const char* psSrc, int nLen, char *psDest);

/**
* @brief Do TDES decryption with specified 'nKeyType' key(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in]  psSrc         Source data
* @param [in]  nLen          Length of data
* @param [out] psDest        Target data
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubUnDes3(const char *psSrc, int nLen, char *psDest);

/**
* @brief Do soft-TDES encryption with 'psKey'.
* @param [in]  psKey         Key string
* @param [in]  psSrc         Source data---8byte
* @param [out] psDest        Target data---8byte
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubSoftDes3(const char *psKey, const char *psSrc, char *psDest);

/**
* @brief Do soft-TDES decryption with 'psKey'.
* @param [in]  psKey         Key string
* @param [in]  psSrc         Source data---8byte
* @param [out] psDest        Target data---8byte
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubSoftUnDes3(const char *psKey, const char *psSrc, char *psDest);

/**
 *@brief 	Calculate 3DES
 *@param[in] nMode		Encryption mode
 *@param[in] nKeyLen     Key length (8, 16 or 24 only)
 *@param[in] psKey		Key buffer
 *@param[in] nSrcLen	length of psSrc
 *@param[in] psSrc	Buffer for data to encrypt
 *@param[out]	psDest	Output data
 *@return
 *@li \ref NAPI_OK "NAPI_OK"  				Success
 *@li \ref EM_NAPI_ERR "EM_NAPI_ERR"		Fail
*/
int PubTDes(int nMode, int nKeyLen, const char* psKey, int nSrcLen, const char* psSrc, char *psDest);

/**
 *@brief 	Calculate KCV
 *@param[in] nKeyLen     Key length (8, 16 or 24 only)
 *@param[in] pszKey		Key buffer
 *@param[out]	pszKcv	Output data
 *@return
 *@li \ref NAPI_OK "NAPI_OK"  				Success
 *@li \ref EM_NAPI_ERR "EM_NAPI_ERR"		Fail
*/
int PubCalcKCV(int nKeyLen, char *pszKey, char *pszKcv);

/**
* @brief Get security library version
* @param [out] pszVer  Version information, 12bytes
* @return 
* @li void
*/
void PubGetSecrityVerion(char *pszVer);

/**
* @brief Display string on pinpad
* @param [in]  pszLine1  String displayed in lin 1
* @param [in]  pszLine2  String displayed in lin 2
* @param [in]  pszLine3  String displayed in lin 3
* @param [in]  pszLine4  String displayed in lin 4
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubDispPinPad(const char *pszLine1, const char *pszLine2, const char *pszLine3, const char *pszLine4);

/**
* @brief Clear the message and display default info on pinpad(for PinPad)
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubClrPinPad(void);

/**
* @brief pinpad beep
* @param [in]  nDuration     time of duration
* @param [in]  Buzzer tone   0 – Default   1 – Alert (750Hz)  2 – Success (1500Hz) 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PubPinpadBeep(int nDuration, int nType);

/**
* @brief show QR Code
* @param [in]  nVersion	Version(defult 0, from 0-40)
* @param [in]  pszBuffer content of display
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PubGenAndShowQr(int nVersion, char *pszBuffer);

/**
* @brief show QR Code
* @param [in]  pszCharaterCode	feature code
* @param [in]  name of signature
* @param [in]  nTimeOut
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PubDoSignatureByPinpad(char *pszCharaterCode, char *pszSignName, int nTimeOut);

/**
* @brief Scan the code through the Pinpad
* @param [out]  results of scanning.
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PubDoScanByPinpad(char *pszBuffer);

/** @}*/ // End of Security
#endif

