#!/bin/bash

# This bash script modifies the Trusted Application UUID of 
# the DebugExtension config file according to the given parameter.
#
# This script also performs the TCP port forwarding


NAME_AES="Aes"
NAME_ROT13="Rot13"
NAME_RSA="Rsa"
NAME_SHA256="Sha256"

UUID_AES="07020000000000000000000000000000"
UUID_ROT13="04010000000000000000000000000000"
UUID_RSA="07040000000000000000000000000000"
UUID_SHA256="06010000000000000000000000000000"

UUID_NEW="00000000000000000000000000000000"

TA_NAMES="$NAME_AES $NAME_ROT13 $NAME_RSA $NAME_SHA256"
# TA_SAMPLES_UUID="$UUID_AES $UUID_ROT13 $UUID_RSA $UUID_SHA256"

DBG_EXT_ROOT=`pwd`
# echo "DBG_EXT_ROOT : $DBG_EXT_ROOT"

TA_CFG_FILE_PATH="/DebugExtension/Boards/Generic/MobiCore_Android/"
TA_CFG_FILE_NAME="trusted_app.rvc"

CFG_FILE_PATH=$DBG_EXT_ROOT$TA_CFG_FILE_PATH$TA_CFG_FILE_NAME
# echo "CFG_FILE_PATH : $CFG_FILE_PATH"

ADB_COMMAND="adb forward tcp:3010 tcp:3010"


usage()
{
   echo " --------------------------------------------------------"
   echo " $0 :"
   echo " It modifies the Debug Extension config file according to"
   echo " the given parameter."
   echo ""
   echo " In every cases, the script does the TCP port forwarding"
   echo " using the adb command"
   echo ""
}

usage_variable()
{
   echo " Usage 1: $0 [Trusted_Application_Name]"
   echo "   [Trusted_Application_Name] could be:"
   echo "   Aes"
   echo "   Rot13"
   echo "   Rsa"
   echo "   Sha256"
   echo ""
   echo " Usage 2: $0"
   echo "   Only performs the TCP port forwarding"
   echo ""
   echo " Usage 3: $0 [UUID]"
   echo "   [UUID] is a custom 32 digits hexadecimal number"
}

file_exists()
{
   if [ ! $1 ]; then
      return 0
   fi

   if [ -f $1 ]; then
      return 1
   else
      return 0
   fi
}

# not to abandon shell
exit_function()
{
   exit 1
}



SCRIPT_PART_1="<?xml version=\"1.0\"?>
<!--Copyright (C) 2009-2011 ARM Limited. All rights reserved.-->
<?RVConfigUtility MajorVersion = \"0\" MinorVersion = \"0\" PatchVersion = \"0\"?>
<RVConfigUtility>
   <RVConfigModule>
      <?RVConfig DLL_NAME = \"connection\" MajorVersion = \"1\" MinorVersion = \"0\" PatchVersion = \"0\" Expanded = \"true\" Active = \"false\" Enabled = \"true\" UserLabel = \"\"?>
      <ScanChainNames Type = \"Str\">?</ScanChainNames>
      <ConnectionManager Type = \"User\" Inherited = \"true\">0x00AE86E4</ConnectionManager>
      <TemplateManager Type = \"User\" Inherited = \"true\">0x00AE8700</TemplateManager>
      <BrokerCoreList Type = \"User\" Inherited = \"true\">0x00AE8734</BrokerCoreList>
      <PlatformName Type = \"Str\" Inherited = \"true\"/>
      <NoOfScanChains Type = \"Int32\" Inherited = \"true\">1</NoOfScanChains>
      <FirmwareVersion Type = \"Str\">4.5.0</FirmwareVersion>
      <ConnectionName Type = \"Str\" Inherited = \"true\"/>
      <ChildModuleName Type = \"Str\" Inherited = \"true\">Devices</ChildModuleName>
      <RVConfigModule>
         <?RVConfig DLL_NAME = \"scanchain\" Expanded = \"true\" Active = \"true\" Enabled = \"true\" UserLabel = \"\"?>
         <TemplateName Type = \"Str\" Inherited = \"true\">00000000000000000000000000000000</TemplateName>
         <ChildModuleName Type = \"Str\" Inherited = \"true\">Trusted_App</ChildModuleName>
         <Class Type = \"Str\">CoreExecutable</Class>
         <CanonicalName Type = \"Str\">Trusted_App</CanonicalName>
         <DeviceOptions Type = \"Str\" Inherited = \"true\"/>
         <IRLength Type = \"Str\" Inherited = \"true\">4</IRLength>
         <RVConfigModule>
            <?RVConfig DLL_NAME = \"dynamiccore\" Expanded = \"true\" Active = \"false\" Enabled = \"true\" UserLabel = \"\"?>
            <TemplateName Type = \"Str\">"

SCRIPT_PART_2="</TemplateName>
            <ModuleName Type = \"Str\">Trusted_App</ModuleName>
            <IRLength Type = \"Str\">4</IRLength>
            <DeviceOptions Type = \"Str\"/>
            <CanonicalName Type = \"Str\">Trusted_App_cn</CanonicalName>
            <Class Type = \"Str\">CoreExecutable</Class>
            <ExtendedTypeInfo Type = \"Branch\"/>
         </RVConfigModule>
         <ModuleName Type = \"Str\">Devices</ModuleName>
         <DataList Type = \"Branch\">
            <DataListUID Type = \"User\">0x00000002</DataListUID>
            <ClockType Type = \"Int32\">7</ClockType>
            <ClockSpeed Type = \"Str\">5000000</ClockSpeed>
            <OtherSpeed Type = \"Str\">5.000 MHz</OtherSpeed>
            <Tap_Count Type = \"UInt32\">1</Tap_Count>
            <Tap_0 Type = \"Branch\">
               <DataListUID Type = \"User\">0x00000005</DataListUID>
               <CoreName Type = \"Str\">Trusted_App_c</CoreName>
               <DllName Type = \"Str\">dynamiccore</DllName>
               <TemplateVersion Type = \"Str\">1:0:0</TemplateVersion>
               <DllVersion Type = \"Str\">rvi.dynamic.core</DllVersion>
               <Description Type = \"Str\">Trusted_App_d</Description>
               <IRLength Type = \"Str\">4</IRLength>
               <TemplateName Type = \"Str\">/real-ice/jtag-templates/arm/arm926ejs/V1.0</TemplateName>
               <DeviceID Type = \"Str\">0x07926F0F</DeviceID>
               <Manufacturer Type = \"Str\">UNKNOWN (Generic ARM)</Manufacturer>
               <DeviceOptions Type = \"Str\"/>
               <Class Type = \"Str\">CoreExecutable</Class>
               <CanonicalName Type = \"Str\">Trusted_App_cn</CanonicalName>
            </Tap_0>
         </DataList>
      </RVConfigModule>
      <ConnectionType Type = \"Str\">TCP/IP</ConnectionType>
      <CoresightAssociations Type = \"Branch\">
         <Device Type = \"EnumStr\">Name=Trusted_App;Type=Trusted_App</Device>
      </CoresightAssociations>
   </RVConfigModule>
</RVConfigUtility>
"

print_script()
{
   file_exists $CFG_FILE_PATH
   ret_code=$?

   if [ $ret_code -eq 1 ]; then
      printf "$SCRIPT_PART_1" > $CFG_FILE_PATH
      printf "$UUID_NEW" >> $CFG_FILE_PATH
      printf "$SCRIPT_PART_2" >> $CFG_FILE_PATH
   else
      printf "$SCRIPT_PART_1"
      printf "$UUID_NEW"
      printf "$SCRIPT_PART_2"
   fi
}

adb_command()
{
   echo "$ADB_COMMAND"
   eval $ADB_COMMAND 
}

set_uuid_from_TA_name()
{
   if [ $1 = "$NAME_AES" ]; then
      UUID_NEW=$UUID_AES
   elif [ $1 = "$NAME_ROT13" ]; then
      UUID_NEW=$UUID_ROT13
   elif [ $1 = "$NAME_RSA" ]; then
      UUID_NEW=$UUID_RSA
   elif [ $1 = "$NAME_SHA256" ]; then
      UUID_NEW=$UUID_SHA256
   fi
}

check_uuid()
{
   str=$1
   str_len=${#str}

   if [ $str_len -ne 32 ]; then
      echo "ERROR : UUID is not a 32 digits hexadecimal number"
      echo "        Argument length is : $str_len"
      return 0
   fi

   if ! [[ $1 =~ ^[0-9A-Fa-f]{32}$ ]] ; then
      echo "ERROR : UUID is not a 32 digits hexadecimal number"
      return 0
   else
      return 1
   fi
}

# check the location of this script
if [ ! -d "$DBG_EXT_ROOT/DebugExtension" ]; then
   echo "ERROR : The script should be in the Tools directory"
   exit_function
fi

# enum values
ta_name_match=1
uuid_match=2

# check parameters
if [ $1 ]; then
   if [ $1 = "help" ] || [ $1 = "--help" ] || [ $1 = "-h" ]; then
     usage
     usage_variable
     exit_function
   else
      case_match=0

      for ta_name in ${TA_NAMES}; do
         if [ $ta_name = $1 ]; then
            case_match=$ta_name_match
            set_uuid_from_TA_name $ta_name
            break
         fi
      done

      if [ $case_match -eq $ta_name_match ]; then
         print_script
         adb_command
         exit_function
      else
         check_uuid $1
         ret_code=$?
         if [ $ret_code -eq 1 ]; then
            UUID_NEW=$1
            case_match=$uuid_match
         fi
      fi

      if [ $case_match -eq $uuid_match ]; then
         print_script
         adb_command
         exit_function
      else
         # error bad argument
         echo "        $1"
         usage_variable
         exit_function
      fi
  fi
else
   adb_command
fi

