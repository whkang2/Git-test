/*
IRBlaster v0.0.1**********************************************
1.First release
IRBlaster v0.1.0**********************************************
1. Added TCFXIRW

...................................................................... version 0.2.0
1. Added send_IR_MEM. Allows sending of ir command via MEM database.
2. fix string comparison and string length issue in MEM database
...................................................................... version 0.2.1
1. Return error_code for unrecognized command.
...................................................................... version 0.2.2
1. Allow ATx and BTx led to flash when sending infrared signal.
...................................................................... version 0.2.3
1. Allow IRBlaster to send IR signal via TIRXSND and TIRBSND. 
...................................................................... version 1.2.3
1. First Beta version released. 
 ***************************************************************************************/



const char this_app_version[] = "1.2.3";


//IRBlaster
#include "IO_Definition.h"
#include "CF_Protocol.h"
#include "CFX.h"
#include <stdio.h>
#include "Conversion.h"
#include "delay.h"
#include <stddef.h> //for NULL
#include "CFLink485.h"
#include <string.h>
#include "RS232.h"
#include "MsgBuilder.h"
#include "IR.h"

static void this_ProcessMsg(S_CFLINK_MSG *msg);

extern CF_DeviceInfo device_info;


void Device_Init(void)
{
	//Add 1 line here from whkang2
	//Add 2 line here from whkang
	LED1_TRIS		
	LED2_TRIS
	LED3_TRIS
	IR_SEL_TRIS		//select between dba (0) or cfir (1)		
	SEL0_TRIS		//mux 8-3 selector pin		
	SEL1_TRIS		//mux 8-3 selector pin
	SEL2_TRIS		//mux 8-3 selector pin
	IRIN_PORT1_TRIS
	IRIN_PORT2_TRIS
	IR8_init(SLOT1);
	//whkang add here
	sprintf(device_info.dev_model,"IRBlaster"); 
	sprintf(device_info.app_version,this_app_version);
	//whkang2 add here
	device_info.lan_id = GetCFLinkLANID();
	device_info.sub_id = GetCFLinkSUBID();
	GetCFLink485Version(device_info.cflink_version);
	//spiflash_read_identification(device_info.spiflash_identification);
	get_serial_number(device_info.serial_number);
	cfx_read_TGT_list();
	
}

/*=====================================================================================
  announce_myself
  Report yourself on power-up. 
  As this is self-reporting msg, message sender type is set to NONE.
  =====================================================================================*/
void Device_Announce(void)
{
	S_CFLINK_MSG msg_out;

	cfx_QCFXWHO((char*)msg_out.CFLinkMsgFormat.CF_Data);

	//Broadcast the message.
	build_cf_msg("IRB","WHO",(char*)msg_out.CFLinkMsgFormat.CF_Data,0,&msg_out);
	send_cf_msg(SELF_REPORTING,NULL,&msg_out);

	//send the message to TGT.
	build_cf_msg("IRB","WHO",(char*)msg_out.CFLinkMsgFormat.CF_Data,0,&msg_out);
	send_cf_msg(CFX_TGT_REPORTING,NULL,&msg_out);

}

void Device_RestoreFactory(void)
{
 	UINT8 default_target_list[] = {0xFF,0x00,0x00,0x00,0x00};
	memcpy(device_info.CFX_target_list,default_target_list,sizeof(default_target_list));
	cfx_save_TGT_list();

	SetCFLinkLANID(DEFAULT_LANID);
	SetCFLinkSUBID(DEFAULT_SUBID);
	device_info.lan_id = GetCFLinkLANID();
	device_info.sub_id = GetCFLinkSUBID();

}


static void this_ProcessMsg(S_CFLINK_MSG *msg_in)
{
	UINT16 err_code;
	char reply_cmd[5]="";
	S_CFLINK_MSG msg_in_copy,msg_out;
	
	memset(msg_out.CFLinkBuffer,0,MAX_CFFRAME_LEN);
	memcpy(&msg_in_copy,msg_in,sizeof(S_CFLINK_MSG));
	msg_in_copy.CFLinkBuffer[msg_in_copy.CFLinkMsgLen-2]=0;


	
	if(msg_in->CFLinkMsgFormat.CF_MSGTYPE==QUERY_TYPE_MSG)
	{	
		if(strncmp((char*)msg_in->CFLinkMsgFormat.CF_CMD,"WHO",3)==0)
		{
			/*Rx: QMODWHO
		  	  Tx: RMODWHO
		  	  To query device info
			*/	
			strncpy(reply_cmd,"WHO",3);
			err_code = cfx_QCFXWHO((char*)msg_out.CFLinkMsgFormat.CF_Data);
		}
		else if(strncmp((char*)msg_in->CFLinkMsgFormat.CF_CMD,"SRN",3)==0)
		{
			/*Rx: QMODSRN
			  Tx: RMODSRN
			*/
			strncpy(reply_cmd,"SRN",3);
			err_code = cfx_QCFXSRN((char*)msg_out.CFLinkMsgFormat.CF_Data);
		}
		else if(strncmp((char*)msg_in->CFLinkMsgFormat.CF_CMD,"TGT",3)==0)
		{
			/*Rx: CIOXCFG
		  	  Tx: RIOXCFG
		  	  To configure io module setting
			*/	
			strncpy(reply_cmd,"TGT",3);
			err_code = cfx_QCFXTGT((char*)msg_in_copy.CFLinkMsgFormat.CF_Data,(char*)msg_out.CFLinkMsgFormat.CF_Data);
		}
		else	
		{
			err_code = ERR_CF_CMD_NOT_RECOGNISE;
		}
	
	}
	else if(msg_in->CFLinkMsgFormat.CF_MSGTYPE==CONFIGURATION_TYPE_MSG)
	{
	
		if(strncmp((char*)msg_in->CFLinkMsgFormat.CF_CMD,"DID",3)==0)
		{
			/*Rx: CMODDID
		  	  Tx: RMODDID
		  	  To set device ID
			*/	
			strncpy(reply_cmd,"DID",3);
			err_code = cfx_CCFXDID((char*)msg_in_copy.CFLinkMsgFormat.CF_Data,(char*)msg_out.CFLinkMsgFormat.CF_Data);
		}
		else if(strncmp((char*)msg_in->CFLinkMsgFormat.CF_CMD,"TGT",3)==0)
		{
			/*Rx: CIOXCFG
		  	  Tx: RIOXCFG
		  	  To configure io module setting
			*/	
			strncpy(reply_cmd,"TGT",3);
			err_code = cfx_CCFXTGT((char*)msg_in_copy.CFLinkMsgFormat.CF_Data,(char*)msg_out.CFLinkMsgFormat.CF_Data);
		}
		else
		{
			err_code = ERR_CF_CMD_NOT_RECOGNISE;
		}
	}
	else if(msg_in->CFLinkMsgFormat.CF_MSGTYPE==TRANSMISSION_TYPE_MSG)
	{
	
		if(strncmp((char*)msg_in->CFLinkMsgFormat.CF_CMD,"LDR",3)==0)
		{
			/*Rx:TMODLDR
			  Tx:will not generate reply
			  To go into bootloader mode
			*/
			strcpy(reply_cmd,"LDR");
			err_code = cfx_TCFXLDR(msg_in->sender_type,msg_in->sender_id,(char*)msg_in_copy.CFLinkMsgFormat.CF_Data,(char*)msg_out.CFLinkMsgFormat.CF_Data);
		}
		else if(strncmp((char*)msg_in->CFLinkMsgFormat.CF_CMD,"RST",3)==0)
		{
			/*Rx:TMODLDR
			  Tx:will not generate reply
			  To go into bootloader mode
			*/
			strcpy(reply_cmd,"RST");
			err_code = cfx_TCFXRST((char*)msg_out.CFLinkMsgFormat.CF_Data);
		}
		else if(strncmp((char*)msg_in->CFLinkMsgFormat.CF_CMD,"IRW",3)==0)
		{
			/*Rx:TMODLDR
			  Tx:will not generate reply
			  To go into bootloader mode
			*/
			strcpy(reply_cmd,"IRW");	
			err_code = cfx_TCFXIRW((char*)msg_in_copy.CFLinkMsgFormat.CF_Data,(char*)msg_out.CFLinkMsgFormat.CF_Data);
		}
		else if(strncmp((char*)msg_in->CFLinkMsgFormat.CF_CMD,"SND",3)==0)
		{
			/*Rx:TMODLDR
			  Tx:will not generate reply
			  To go into bootloader mode
			*/
			strcpy(reply_cmd,"SND");	
			err_code = ir_TIRXSND((char*)msg_in_copy.CFLinkMsgFormat.CF_Data,(char*)msg_out.CFLinkMsgFormat.CF_Data);
			//err_code = cfx_TCFXIRW(msg_in_copy.CF_Data,msg_out.CF_Data);
		}
		else
		{
			err_code = ERR_CF_CMD_NOT_RECOGNISE;
		}

	}
	else
	{
		err_code = ERR_CF_CMD_NOT_RECOGNISE;
	}

	if(err_code==COMPLETION_WITHOUT_ERR)
	{
		build_cf_msg("IRB",reply_cmd,(char*)msg_out.CFLinkMsgFormat.CF_Data,0,&msg_out);
		send_cf_msg(msg_in->sender_type,msg_in->sender_id,&msg_out);
	}
	else if(err_code==NO_RETURN_MSG)
	{
		//Don't have to handle here.
	}
	else
	{
		build_ERR_msg("IRB",err_code,msg_in,&msg_out);
		send_cf_msg(msg_in->sender_type,msg_in->sender_id,&msg_out);
	}

}
void Device_ProcessMsg(S_CFLINK_MSG *msg_in)
{
	char product_code[4]= "";
	
	msg_in->CFLinkBuffer[msg_in->CFLinkMsgLen-2]=0;//null-terminate cf_data.
	
	if(msg_in->CFLinkMsgFormat.CF_MSGTYPE==REPLY_TYPE_MSG)
	return;

	memcpy(product_code,msg_in->CFLinkMsgFormat.CF_TYPE,3);
	
	if(strncmp(product_code,"CFX",3)==0||\
	   strncmp(product_code,"IRB",3)==0)
	{
		this_ProcessMsg(msg_in);
	}
	else if(strncmp(product_code,"IRX",3)==0)
	{
		IR_ProcessMsg(msg_in);
	}
	else
	{
		//Received a non broadcast message will not generate error for unrecognized command.
		//For example:
		//Received broadcast of TLANXXX will not generate error reply.
		//Received targetted of TLANXXX will have to generate error reply.
		if(msg_in->CFLinkMsgFormat.ID !=BROADCAST)
		{
			S_CFLINK_MSG msg_out;
			UINT16 err_code;
		
			err_code = ERR_CF_CMD_NOT_RECOGNISE;	
			memset(msg_out.CFLinkBuffer,0,MAX_CFFRAME_LEN);
		
			build_ERR_msg("IRB",err_code,msg_in,&msg_out);
			send_cf_msg(msg_in->sender_type,msg_in->sender_id,&msg_out);
		}


	}
}




void Device_ProcessEvent(void)
{
	if(IRIN_PORT1==0)
	{
		//learn_38khz();
	}

	if(IRIN_PORT2==0)
	{		
		//learn_38khz();
	}

	if(device_info.switch_event.setup_pressed)
	{

	}
	
	device_info.switch_event.setup_pressed 	= 0;
}
