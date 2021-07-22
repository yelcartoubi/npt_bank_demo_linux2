/**************************************************************************
* Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
*
* @file  xml.c
* @brief consider the limit of memory(MPOS), we load XML configuraion one line by on line
* @version 1.0
* @author linld
* @date 2019-9-25
**************************************************************************/


#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

#define YES '1'
#define NO '0'


#define NODE_CONFIG "config"
#define NODE_ENTRY  "entry"

#define NODE_DRL  "DRL"
#define DRL_TYPE_PAYWAVE 1 
#define DRL_TYPE_AMEX    2

typedef struct
{
	uchar Transtype; 					/**The Buffer Of The Aid, 9F06*/
	int nTLVListLen;
	uchar szTLVList[200];
	
}PaypassTranTypeAID;

static int TrasTypeCheck = 0;
static PaypassTranTypeAID pTrasType_Check[10] = {{0}};

static int ParseAID(char *szLineBuf, char *szAidTlv, int *nTlvLen);
static int ParseCapk(char *szLineBuf, L3_CAPK_ENTRY *stCapk);

/**
* @brief read one line from file
* 		 [fgets] only get one line which is end with "0A"/"0D 0A" ,
* 		 But, sometimes one line is end with "0D", so we make this api.
* @params 
* @return 0
* @author linld
* @date 2019-9-26
*/

static int ReadLine(char *str, int n, int nFd)
{
	int num = 0;
	int index = 0;
	
	while(1)
	{
		num = PubFsRead(nFd, str+index, 1);
//	    num = fread(str+index, sizeof(char), 1, stream);
		if (num < 1)
		{
			break;
		}

		
		if (*(str+index) == 0x0D 
			|| *(str+index) == 0x0A)
		{
			if (index == 0) //empty line
			{
			    continue;
			}
			else
			{
			    *(str+index) = 0x00;
			}
			
		    return 0;
		}
	

		index++;
	}

	return 1;
    
}


static int Invaild(char *lineStr)
{
//	if (strlen(lineStr) == 1 || (lineStr[0] == 0x0d && lineStr[1] == 0x0a))//Empty line
//	{
//	    return YES;
//	}

	if (strstr(lineStr, "<!--") != NULL)//Comments
	{
	    return YES;
	}

	if (strstr(lineStr, "<?") != NULL) //head
	{
	    return YES;
	}

	return NO;
    
}


static int IsNode(char *node, char *str, char *lineStr)
{
	char *p1 = NULL;
	char *p2 = NULL;
	
	char szContent[100] = {0};
	
    if (NULL == strstr(lineStr, node))
    {
        return NO;
    }

	p1 = strchr(lineStr, '\"');
	if (p1 == NULL)
	{
	    return NO;
	}

	p2 = strchr(p1+1, '\"');
	if (p2 == NULL)
	{
	    return NO;
	}

	memcpy(szContent, p1+1, p2-p1-1);
	
	PubAllTrim(szContent);

	if (strlen(szContent) != strlen(str))
	{
	    return NO;
	}

	if (0 == memcmp(szContent, str, strlen(str)))
	{
	    return YES;
	}

	return NO;
}


static int IsNodeEnd(char *node, char *lineStr)
{
	char szTmp[50] = {0};
	
	sprintf(szTmp, "</%s", node);
	
    if (strstr(lineStr, szTmp) != NULL)
	{
	    return YES;
	}
	return NO;
}

static int IsElement(char *lineStr)
{
    if (strstr(lineStr, "<item") != NULL)
	{
	    return YES;
	}
	return NO;
}

static int IsDrlElement(char * lineStr, uchar * drlType)
{
    if (strstr(lineStr, "PAYWAVE DRL") != NULL)
	{
		*drlType = DRL_TYPE_PAYWAVE;
  		return YES;
	}
	else if(strstr(lineStr, "AMEX DRL") != NULL)
	{
		*drlType = DRL_TYPE_AMEX;
		return YES;
	}
	return NO;
}

static int GetAttribute(char *attr, char *out, char *lineStr)
{
	char *p = NULL;
	char *p1 = NULL;
	char *p2 = NULL;

	p = strstr(lineStr, attr);
    if (p == NULL)
    {
        return -1;
    }

	p1 = strchr(p, '\"');
	if (p1 == NULL)
	{
	    return -2;
	}

	p2 = strchr(p1+1, '\"');
	if (p2 == NULL)
	{
		return -3;    
	}

	memcpy(out, p1+1, p2-p1-1);
	PubAllTrim(out);
	return 0;
	
}

static void DispLoadCfgInfo(char cCardInterface, char cType)
{
	char szContent[32] = {0};

	PubClearAll();
	if (cType == 2) // capk
	{
		sprintf(szContent, "%s", tr("LOAD CAPK"));
	}
	else
	{
		if (cCardInterface == L3_CONTACT)
		{
			sprintf(szContent, "%s", tr("LOAD CT AID"));
		}
		else
		{
			sprintf(szContent, "%s", tr("LOAD CTL AID"));
		}
	}

	PubDisplayStrInline(0, 2, szContent);
	PubDisplayStrInline(0, 4, tr("PLEASE WAIT..."));
	PubUpdateWindow();
}

int SetPayWaveDrl(char * szLineBuf, STDRLPARAM * strPayWaveDrl)
{
	char szKey[30] = {0};
	int nRet = 0;
	char szValue[80] = {0};
	char szHexVal[40] = {0};
	int nValLen = 0;
		
	memset(szKey, 0, sizeof(szKey));
	nRet = GetAttribute("key", szKey, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

	memset(szValue, 0, sizeof(szValue));
	nRet = GetAttribute("value", szValue, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

	TRACE("[%s]:[%s]", szKey, szValue);
	nValLen = strlen(szValue);
	memset(szHexVal, 0, sizeof(szHexVal));
	PubAscToHex((uchar *)szValue, nValLen, 1, (uchar * )szHexVal);
	nValLen = (nValLen+1)>>1;

	if (0 == strcmp(szKey, "APP ID"))
	{
		strPayWaveDrl->ucDrlAppIDLen = nValLen;
		memcpy(strPayWaveDrl->usDrlAppId, szHexVal, strPayWaveDrl->ucDrlAppIDLen);		
	} 
	else if (0 == strcmp(szKey, "ClLimitExist"))
	{
		strPayWaveDrl->ucClLimitExist = (szHexVal[0] & 0xFF);
	}
	else if (0 == strcmp(szKey, "Clss Transaction Limit"))
	{
		memcpy(strPayWaveDrl->usClTransLimit , szHexVal, 6);
	}
	else if (0 == strcmp(szKey, "Clss Offline Limit"))
	{
		memcpy(strPayWaveDrl->usClOfflineLimit , szHexVal, 6);
	}
	else if (0 == strcmp(szKey, "Cvm Limit"))
	{
		memcpy(strPayWaveDrl->usClCvmLimit , szHexVal, 6);	
	}

	return 0;
}


int SetAmexDrl(char * szLineBuf, STEXDRLPARAM * strAmexDrl)
{
	char szKey[30] = {0};
	int nRet = 0;
	char szValue[60] = {0};
	char szHexVal[30] = {0};
	int nValLen = 0;	
	
	memset(szKey, 0, sizeof(szKey));
	nRet = GetAttribute("key", szKey, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

	memset(szValue, 0, sizeof(szValue));
	nRet = GetAttribute("value", szValue, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

	TRACE("[%s]:[%s]", szKey, szValue);
	nValLen = strlen(szValue);
	memset(szHexVal, 0, sizeof(szHexVal));
	PubAscToHex((uchar *)szValue, nValLen, 1, (uchar * )szHexVal);
	
	if (0 == strcmp(szKey, "DRL Exist"))
	{
		strAmexDrl->ucDrlExist = atoi(szValue);
	} 
	else if (0 == strcmp(szKey, "DRL ID"))
	{
	   strAmexDrl->ucDrlId = (szHexVal[0] & 0xFF);
	}
	else if (0 == strcmp(szKey, "Limist Exist"))
	{
		strAmexDrl->ucClLimitExist = (szHexVal[0] & 0xFF);
	}
	else if (0 == strcmp(szKey, "Clss Transaction Limit"))
	{
		memcpy(strAmexDrl->usClTransLimit , szHexVal, 6);
	}
	else if (0 == strcmp(szKey, "Clss Offline Limit"))
	{
		memcpy(strAmexDrl->usClOfflineLimit , szHexVal, 6);
	}
	else if (0 == strcmp(szKey, "Cvm Limit"))
	{
		memcpy(strAmexDrl->usClCvmLimit , szHexVal, 6);	
		
	}

	return 0;
}

int AddDrlOne(uchar * ucDrlTlv, STDRLPARAM * strPayWaveDrl, STEXDRLPARAM * strAmexDrl, uchar ucType, int * nDrlOffset)
{
	int nDrlstrLen = 0;
	TRACE("[nDrlOffset] = %d", *nDrlOffset);
	
	if(ucType == DRL_TYPE_PAYWAVE)
	{		
		TRACE_HEX(strPayWaveDrl, sizeof(STDRLPARAM), "[Paywave drl add]: ");
		
		nDrlstrLen = sizeof(STDRLPARAM);
		memcpy(ucDrlTlv + *nDrlOffset, strPayWaveDrl, nDrlstrLen);
		*nDrlOffset += nDrlstrLen;
		memset(strPayWaveDrl, 0, sizeof(STDRLPARAM));
	}
	else if(ucType == DRL_TYPE_AMEX)
	{
		TRACE_HEX(strAmexDrl, sizeof(STEXDRLPARAM), "[Amex drl add]: ");
		
		nDrlstrLen = sizeof(STEXDRLPARAM);
		memcpy(ucDrlTlv + *nDrlOffset, strAmexDrl, nDrlstrLen);		
		*nDrlOffset += nDrlstrLen;
		memset(strAmexDrl, 0, sizeof(STEXDRLPARAM));
	}

	return 0;
}

int AddDrlAll(uchar * ucDrlTlv, char *szAidTlv, int *nTlvLen, int nDrlOffset, uchar ucDrlType)
{
	TRACE("[nDrlOffset] = %d:", nDrlOffset);
	
	if (ucDrlType == DRL_TYPE_PAYWAVE)
	{
		TRACE_HEX(ucDrlTlv, nDrlOffset, "Paywave Drl [DF3F]: ");
		TlvAddEx((uchar*)"DF3F", nDrlOffset, (char*)ucDrlTlv, szAidTlv, nTlvLen);
	}
	else if (ucDrlType == DRL_TYPE_AMEX)
	{
		TRACE_HEX(ucDrlTlv, nDrlOffset, "Amex Drl [DF53]: ");
		TlvAddEx((uchar*)"DF53", nDrlOffset, (char*)ucDrlTlv, szAidTlv, nTlvLen);
	}
	
	return 0;
}

int DealPaypassTranTypeAID(char *szAidTlv)
{
	int i = 0;
	char szNewAidTlv[1500] = {0};
	int nTlvLen = 0;
	int nRet = 0;

	for(i=0;i<TrasTypeCheck;i++)
	{
		switch(pTrasType_Check[i].Transtype)
		{
			case 0x09:
				TlvAddEx((uchar*)"1F8101", 2, "0X01", szNewAidTlv, &nTlvLen);
				TlvAddEx((uchar*)"DF7D", 2, "0x09", szNewAidTlv, &nTlvLen);
				//TlvAddEx(pTrasType_Check[i].ucTag, pTrasType_Check[i].nVaulelen, pTrasType_Check[i]->TagValue, szNewAidTlv, &nTlvLen);
				strcat(szNewAidTlv, (char*)pTrasType_Check[i].szTLVList);
				nTlvLen += strlen((char*)pTrasType_Check[i].szTLVList);
				TRACE_HEX( szNewAidTlv, nTlvLen, "szNewAidTlv: ");
				
				nRet = NAPI_L3LoadAIDConfig(0x02, NULL, (uchar *)szNewAidTlv, &nTlvLen, CONFIG_UPT);
				TRACE("NAPI_L3LoadAIDConfig->CONFIG_UPT, nRet=%d", nRet);
				break;
			default:
				break;
		}

	}

	return 0;

}


/**
* @brief Load configuration from XML file
* @param NONE
* @return
* @li 0  success
* @li < 0 fail
* @author linld
* @date 2019-9-26
*/
int LoadXMLConfig(void)
{
	char szLineBuf[1000] = {0};
	L3_CARD_INTERFACE cardInterface = 0;
	int nIsTerminalConfig = NO;
	int nConfigType = 0;
	int nRet = 0, nFd = 0, nEnd = 0;
	char szAidTlv[1500] = {0};
	int nTlvLen = 0;
	L3_CAPK_ENTRY stCapk;
	STDRLPARAM strPayWaveDrl;
	STEXDRLPARAM strAmexDrl;
	uchar ucDrlIsSetting = 0;
	uchar ucDrlType = 0;
	int nTagOffset = 0;
	int nDrlOffset = 0;
	uchar uCustagTlv[280] = {0};
	uchar ucDrlTlv[500] = {0};

	nFd = PubFsOpen(XML_CONFIG, "r");
	if (nFd < 0)	
	{
	    TRACE("open file[%s] failed[%d].", XML_CONFIG, nFd);
		return -1;
	}

	PubClearAll();
	PubDisplayStrInline(0, 2, tr("LOAD CONFIG"));
	PubDisplayStrInline(0, 4, tr("PLEASE WAIT..."));
	PubUpdateWindow();

	TxnL3LoadAIDConfig(L3_CONTACT, NULL, NULL, NULL, CONFIG_FLUSH);
	TxnL3LoadAIDConfig(L3_CONTACTLESS, NULL, NULL, NULL, CONFIG_FLUSH);
	TxnL3LoadCAPK(NULL, CONFIG_FLUSH);

	memset(&strPayWaveDrl, 0, sizeof(STDRLPARAM));
	memset(&strAmexDrl, 0, sizeof(STEXDRLPARAM));
	memset(uCustagTlv, 0, sizeof(uCustagTlv));
	memset(ucDrlTlv, 0, sizeof(ucDrlTlv));
	memset(szAidTlv, 0, sizeof(szAidTlv));
	memset(&stCapk, 0, sizeof(stCapk));
	
	while (1)
	{
		if (nEnd == 1)
		{
		    break;
		}
		memset(szLineBuf, 0, sizeof(szLineBuf));

	    nRet = ReadLine(szLineBuf, sizeof(szLineBuf), nFd);
		if (nRet == 1)
		{
		    nEnd = 1;
		}
		
//		TRACE("%d[%s]", nLine++, szLineBuf);
		
		if (YES == Invaild(szLineBuf))
		{
		    continue;
		}

		if (YES == IsNode(NODE_CONFIG, "CONTACT", szLineBuf))
		{
			nConfigType = 1;
			cardInterface = L3_CONTACT;
			DispLoadCfgInfo(cardInterface, nConfigType);
			continue;
		}

		if (YES == IsNode(NODE_CONFIG, "CONTACTLESS", szLineBuf))
		{
			nConfigType = 1;
			cardInterface = L3_CONTACTLESS;
			DispLoadCfgInfo(cardInterface, nConfigType);
			continue;
		}

		if (YES == IsNode(NODE_CONFIG, "PublicKeys", szLineBuf))
		{
			nConfigType = 2;
			DispLoadCfgInfo(cardInterface, nConfigType);
			continue;
		}

		if (YES == IsNode(NODE_ENTRY, "Terminal Configuration", szLineBuf))
		{
		    nIsTerminalConfig = YES;
			continue;
		}

		if(YES == IsDrlElement(szLineBuf, &ucDrlType))
		{
			ucDrlIsSetting = 1;
			continue;
		}
		if (YES == IsElement(szLineBuf))
		{
			if (ucDrlIsSetting)
			{
				if (ucDrlType == DRL_TYPE_PAYWAVE)
				{		
					SetPayWaveDrl(szLineBuf, &strPayWaveDrl);
				}
				else if(ucDrlType == DRL_TYPE_AMEX)
				{
					SetAmexDrl(szLineBuf, &strAmexDrl);
				}
				continue;
				
			}

			if (nConfigType == 1) //AID
			{
				nRet = ParseAID(szLineBuf, szAidTlv, &nTlvLen);
			}
			else //CAPK
			{
				nRet = ParseCapk(szLineBuf, &stCapk);
			}
			
			if (nRet != 0)
			{
			    PubFsClose(nFd);
				return nRet;
			}

			continue;
			
		}

		if (YES == IsNodeEnd(NODE_DRL, szLineBuf))
		{
			AddDrlOne(ucDrlTlv, &strPayWaveDrl, &strAmexDrl, ucDrlType, &nDrlOffset);
			ucDrlIsSetting = 0;
			continue;
		}

		if (YES == IsNodeEnd(NODE_ENTRY, szLineBuf))
		{
			if(nTagOffset > 0)
			{
				TRACE("nTagOffset = %d", nTagOffset);
				TRACE_HEX(uCustagTlv, nTagOffset, "Custom Tag [1F811F]: ");
				TlvAddEx((uchar*)"1F811F", nTagOffset, (char*)uCustagTlv, szAidTlv, &nTlvLen);
				nTagOffset = 0;
			}
		
			if (nDrlOffset > 0)
			{
				AddDrlAll(ucDrlTlv, szAidTlv, &nTlvLen, nDrlOffset, ucDrlType);
				nDrlOffset = 0;
				ucDrlIsSetting = 0;
			}
			
			if (nConfigType == 1) 
			{
				if (YES == nIsTerminalConfig)
				{
					nRet = TxnL3LoadTerminalConfig(cardInterface, (uchar *)szAidTlv, &nTlvLen, CONFIG_UPT);
					nIsTerminalConfig = NO;
				}
				else
				{
					DealPaypassTranTypeAID(szAidTlv);
					nRet = TxnL3LoadAIDConfig(cardInterface, NULL, (uchar *)szAidTlv, &nTlvLen, CONFIG_UPT);
				}
				if (nRet != 0) {
					TRACE("nRet = %d", nRet);
				}
				memset(szAidTlv, 0, sizeof(szAidTlv));
				memset(uCustagTlv, 0, sizeof(uCustagTlv));
				memset(ucDrlTlv, 0, sizeof(ucDrlTlv));
				nTlvLen = 0;
				
			}
			else //CAPK
			{
				nRet = TxnL3LoadCAPK(&stCapk, CONFIG_UPT);
				if (nRet != 0) {
					TRACE("nRet = %d", nRet);
				}
		 		memset(&stCapk, 0, sizeof(stCapk));
			}
			continue;
		}

	}

	PubFsClose(nFd);
	return 0;
}

static int ParseAID(char *szLineBuf, char *szAidTlv, int *nTlvLen)
{
	int nRet;
	char szTag[10];
	char szValue[500] = {0};
	char szHexVal[250] = {0};
	int nValLen = 0;

	memset(szTag, 0, sizeof(szTag));
	nRet = GetAttribute("tag", szTag, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}
	
	memset(szValue, 0, sizeof(szValue));
	nRet = GetAttribute("value", szValue, szLineBuf);
	if (nRet != 0)
	{
		TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

//	TRACE("[%s]:[%s]", szTag, szValue);
	nValLen = strlen(szValue);
	memset(szHexVal, 0, sizeof(szHexVal));
	PubAscToHex((uchar *)szValue, nValLen, 1, (uchar * )szHexVal);
	nValLen = (nValLen+1)>>1;
	TlvAddEx((uchar *)szTag, nValLen, szHexVal, szAidTlv, nTlvLen);

	return 0;
	
}

static int ParseCapk(char *szLineBuf, L3_CAPK_ENTRY *stCapk)
{
	int nRet;
	char szKey[50] = {0};
	char szValue[500] = {0};
	char szHexVal[250] = {0};
	int nValLen = 0;

	memset(szKey, 0, sizeof(szKey));
    nRet = GetAttribute("key", szKey, szLineBuf);
	if (nRet != 0)
	{
	    TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}
	
	memset(szValue, 0, sizeof(szValue));
	nRet = GetAttribute("value", szValue, szLineBuf);
	if (nRet != 0)
	{
	    TRACE("GetAttribute fail, nRet=%d", nRet);
		return nRet;
	}

//	TRACE("[%s]:[%s]", szKey, szValue);
	nValLen = strlen(szValue);
	memset(szHexVal, 0, sizeof(szHexVal));
	PubAscToHex((uchar *)szValue, nValLen, 1, (uchar * )szHexVal);
	nValLen = (nValLen+1)>>1;
	
	if (0 == strcmp(szKey, "RID"))
	{
	    memcpy(stCapk->rid, szHexVal, 5);
	} 
	else if (0 == strcmp(szKey, "Index"))
	{
	    stCapk->index = (szHexVal[0] & 0xFF);
//		TRACE("stCapk.index=0x%X", stCapk.index);
	}
	else if (0 == strcmp(szKey, "Hash"))
	{
	    memcpy(stCapk->hashValue, szHexVal, 20);
	}
	else if (0 == strcmp(szKey, "Exponent"))
	{
	    if (0 == memcmp(szValue, "03", 2))
	    {
	        stCapk->pkExponent[0] = 0x00;
			stCapk->pkExponent[1] = 0x00;
			stCapk->pkExponent[2] = 0x03;
	    }
		else
		{
		    memcpy(stCapk->pkExponent, szHexVal, 3);
		}
	}
	else if (0 == strcmp(szKey, "Modulus"))
	{
		stCapk->pkModulusLen = nValLen;
	    memcpy(stCapk->pkModulus, szHexVal, stCapk->pkModulusLen);
	}
	else if (0 == strcmp(szKey, "Hash Algorithm"))
	{
		stCapk->hashAlgorithmIndicator = atoi(szValue);
//		TRACE("stCapk.hashAlgorithmIndicator=%d", stCapk.hashAlgorithmIndicator);
	}
	else if (0 == strcmp(szKey, "Sign Algorithm"))
	{
		stCapk->pkAlgorithmIndicator = atoi(szValue);
	}

	return 0;
}



