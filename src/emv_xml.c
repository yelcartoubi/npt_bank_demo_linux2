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

#define YES 1
#define NO 0


#define NODE_CONFIG "config"
#define NODE_ENTRY  "entry"


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
//    FILE *fp = NULL;
	char szLineBuf[1000] = {0};
	int nAidInterface = 0;
	int nIsTerminalConfig = NO;
	int nConfigType = 0;
	int nRet = 0;
	char szAidTlv[1500] = {0};
	int nTlvLen = 0;
	L3_CAPK_ENTRY stCapk;
	int nFd = 0;
	int nEnd = 0;
		
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


	NAPI_L3LoadAIDConfig(L3_CONTACT, NULL, NULL, NULL, CONFIG_FLUSH);
	NAPI_L3LoadAIDConfig(L3_CONTACTLESS, NULL, NULL, NULL, CONFIG_FLUSH);
	NAPI_L3LoadCAPK(NULL, CONFIG_FLUSH);

	nTlvLen = 0;
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
			nAidInterface = L3_CONTACT;
			continue;
		}

		if (YES == IsNode(NODE_CONFIG, "CONTACTLESS", szLineBuf))
		{
			nConfigType = 1;
			nAidInterface = L3_CONTACTLESS;
			continue;
		}

		if (YES == IsNode(NODE_CONFIG, "PublicKeys", szLineBuf))
		{
			nConfigType = 2;
			continue;
		}

		if (YES == IsNode(NODE_ENTRY, "Terminal Configuration", szLineBuf))
		{
		    nIsTerminalConfig = YES;
			continue;
		}


		if (YES == IsElement(szLineBuf))
		{
//			TRACE("nConfigType=%d", nConfigType);

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

		if (YES == IsNodeEnd(NODE_ENTRY, szLineBuf))
		{
//			TRACE("nConfigType=%d", nConfigType);
			if (nConfigType == 1) //AID
			{
//				TRACE("nAidInterface=%d", nAidInterface);
//				TRACE("nIsTerminalConfig=%d", nIsTerminalConfig);

				TRACE_HEX(szAidTlv, nTlvLen, "TLV CONFIG:");
				
				if (YES == nIsTerminalConfig)
				{
				   nRet = NAPI_L3LoadTerminalConfig(nAidInterface, (uchar *)szAidTlv, &nTlvLen, CONFIG_UPT);
//				   TRACE("NAPI_L3LoadTerminalConfig->CONFIG_UPT, nRet=%d", nRet);

				   nIsTerminalConfig = NO;
				}
				else
				{
				   nRet = NAPI_L3LoadAIDConfig(nAidInterface, NULL, (uchar *)szAidTlv, &nTlvLen, CONFIG_UPT);
//				   TRACE("NAPI_L3LoadAIDConfig->CONFIG_UPT, nRet=%d", nRet);
				}
				memset(szAidTlv, 0, sizeof(szAidTlv));
				nTlvLen = 0;
				
			}
			else //CAPK
			{
				nRet = NAPI_L3LoadCAPK(&stCapk, CONFIG_UPT);
//				TRACE("NAPI_L3LoadCAPK->CONFIG_UPT, nRet=%d", nRet);
				
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



