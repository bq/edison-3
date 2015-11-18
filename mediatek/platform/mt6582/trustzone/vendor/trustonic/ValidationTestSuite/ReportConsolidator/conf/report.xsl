<?xml version="1.0" encoding="ISO-8859-1"?>

<!-- Copyright (c) 2013 TRUSTONIC LIMITED                                      -->
<!-- All rights reserved                                                       -->
<!--                                                                           -->
<!--  The present software is the confidential and proprietary information of  -->
<!--  TRUSTONIC LIMITED. You shall not disclose the present software and shall -->
<!--  use it only in accordance with the terms of the license agreement you    -->
<!--  entered into with TRUSTONIC LIMITED. This software may be subject to     -->
<!--  export or import laws in certain countries.                              -->
 
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:param name="gGroupName"/>
<xsl:param name="gTestName"/>
<xsl:param name="gPage"/>

<!--Main page of test report-->
<xsl:variable name="htmlPageName" select="'index.html'" />
<!-- link to Jira, needs to be filled with the id of the bug -->
<xsl:variable name="linkJira" select="'http://jira.trustonic.internal/jira/browse/'"/>
<!-- link to testplan
<xsl:variable name="pathDoc" select="'./INTEGRATION.html/'"/> -->

<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
<!-- #######                            GLOBAL RENDERING                                ####### -->
<!-- ########################################################################################## -->
<!-- ########################################################################################## -->

<xsl:template match="/">
   <table>
   <tr>
   <td align="center">
      <xsl:if test="$gPage = 'compare'">
         <h1>Results comparison</h1>
      </xsl:if> 
      <xsl:if test="$gPage = 'failures'">
         <h1>All failures</h1>
      </xsl:if> 
      <xsl:if test="$gGroupName = '' and $gTestName = '' and $gPage = ''">
         <h1>Consolidated Test Report</h1>
      </xsl:if> 
      <xsl:if test="$gGroupName != '' and $gTestName = '' and $gPage = ''">
         <h1>Test Report: <xsl:value-of select="$gGroupName"/></h1>
      </xsl:if> 
      <xsl:if test="$gGroupName != '' and $gTestName != '' and $gPage = ''">
         <h1>Test Report: <xsl:value-of select="concat($gGroupName, '.', $gTestName)"/></h1>
      </xsl:if> 
   </td>
   </tr>
   
   <tr>
   <td align="right">
      <img width="250" src="conf/TrustonicLogo.jpg"/>
   </td>
   </tr>
   
   <tr>
   <td>
      <br/><br/>
   </td>
   </tr>

   <xsl:if test="$gGroupName='' and $gTestName='' and $gPage = ''">
      <xsl:variable name="totalTC" select="count(/TestReports/test_list/group/test/test_case)"/>
      <xsl:variable name="totalTCSuccess" select="count(/TestReports/test_list/group/test/test_case[@result='SUCCESS'])"/>
      <xsl:variable name="totalTCFailure" select="count(/TestReports/test_list/group/test/test_case[@result='FAILURE'])"/>
      <xsl:variable name="totalTCWarning" select="count(/TestReports/test_list/group/test/test_case[@result='SHOULD_FAIL_BUT_SUCCESS'])"/>
      <xsl:variable name="totalTCNotExecuted" select="count(/TestReports/test_list/group/test/test_case[@result='NOT_EXECUTED'])"/>
      <xsl:variable name="totalTCKnownFailureMinor" select="count(/TestReports/test_list/group/test/test_case[@result='KNOWN_FAILURE' and @severity='minor'])"/>
      <xsl:variable name="totalTCKnownFailureMajor" select="count(/TestReports/test_list/group/test/test_case[@result='KNOWN_FAILURE' and @severity='major'])"/>
      <xsl:variable name="totalTCKnownFailureCritical" select="count(/TestReports/test_list/group/test/test_case[@result='KNOWN_FAILURE' and @severity='critical'])"/>
      
      
      <xsl:variable name="totalTCAllSuccess" select="$totalTCSuccess+$totalTCWarning"/>
      
      <tr align="left">
      <td>
         <h3>Summary</h3>               

         <table align="left">      

            <tr class="smallNeutral">    
               <td><b>Testcases </b>  </td>
               <td/>
               <td align="right"><b><xsl:value-of select="$totalTC"/> </b></td>           
            </tr>
            <tr class="smallNeutral">                   
               <td>Not executed </td>
               <td/>
               <td align="right"><xsl:value-of select="$totalTCNotExecuted"/></td>                                     
            </tr>
            <tr class="smallGreen">
               <td>Success </td> 
               <td/>                          
               <td align="right"><xsl:value-of select="$totalTCAllSuccess"/></td>
            </tr>           
            
            <tr class="smallOrange">
               <td>Expected failures: </td>
               <td/>
               <td/>
            </tr>  
            <tr class="smallOrange">
               <td/>
               <td>- Minor</td>
               <td align="right"><xsl:value-of select="$totalTCKnownFailureMinor"/></td>
            </tr>  
            <tr class="smallOrange">
               <td/>
               <td>- Major </td>
               <td align="right"><xsl:value-of select="$totalTCKnownFailureMajor"/></td>
            </tr>        
            <tr class="smallRed">
               <td/>
               <td>- Critical </td>
               <td align="right"><xsl:value-of select="$totalTCKnownFailureCritical"/></td>
            </tr>    
            
            <tr class="smallRed">
               <td>Unexpected failures </td>
               <td/>
               <td align="right"><xsl:value-of select="$totalTCFailure"/></td>
            </tr>       
         </table>
      </td>
      </tr>
      
      <!-- only if multi report-->
      <xsl:if test="count(/TestReports/test_list) > 1">
         <td align="left">
            <a>
               <xsl:attribute name="href">
                  <xsl:value-of select="concat($htmlPageName,'?page=compare')" />
               </xsl:attribute>
               Compare results
            </a>
         </td>
      </xsl:if> 
      
      <xsl:if test="$totalTC != $totalTCSuccess">
         <xsl:call-template name="display_failure_link"/>
      </xsl:if> 
      
   </xsl:if> 
   <tr>
   <td>
      <br/><br/>
   </td>
   </tr>
     
   <tr align="center">
   <td>   
      <!-- SUMMARY -->   
      <xsl:if test="$gPage = 'compare'">
         <xsl:call-template name="display_compare_table"/>
      </xsl:if>  
      <xsl:if test="$gPage = 'failures'">
         <xsl:call-template name="display_failures_table"/>
      </xsl:if> 
      <xsl:if test="$gGroupName = '' and $gTestName = ''and $gPage = ''">
         <xsl:call-template name="display_group_table"/>
      </xsl:if> 
      <xsl:if test="$gGroupName != '' and $gTestName = ''and $gPage = ''">
         <xsl:call-template name="display_test_table"/>
      </xsl:if> 
      <xsl:if test="$gGroupName != '' and $gTestName != ''and $gPage = ''">
         <xsl:call-template name="display_test_case_table"/>
      </xsl:if> 
   </td>
   </tr>
   
   <tr>
   <td>
      <br/>
   </td>
   </tr>

   <xsl:if test="$gPage = ''">
      <tr align="center">
      <td>
         <!-- LEGEND -->
         <table class="smallNeutral">
         <tr>
            <td style="padding-right: 50px">
               <h3>Metric Legend</h3>
            </td>
            <td align="right">
               <h3>Result Legend</h3>
            </td>
         </tr>   
         
         <tr>
            <td style="padding-right: 50px">   
               Testcases Total
               <br/>
               <div  class="smallGreen">
                      Testcases Success
               </div>
               <div class="smallRed">
                  Testcases Failure ( Documented failures )
               </div>
            </td>

            <td>
               <table class="smallneutral">
               <tr>
                  <td class="smallGreen" align="left">
                     <img src="conf/greenbullet.gif"/>
                     pass
                  </td>
                  <td>:</td>
                  <td align="right">
                     The result of the test is the expected result
                  </td>
               </tr>
               <tr>
                  <td class="smallRed" align="left">
                     <img src="conf/redbullet.gif"/>
                     fail
                  </td>
                  <td>:</td>
                  <td align="right">
                     The result of the test is not the expected result
                  </td>
               </tr>
               <tr>
                  <td align="center">
                     <img src="conf/failed.gif"/>
                  </td>
                  <td>:</td>
                  <td align="right">
                     The test is not executed
                  </td>
               </tr>
               </table>
            </td>
         </tr>
         </table>
      </td>
      </tr>
   </xsl:if>
      
   <tr>
   <td>
      <br/>
      <br/>
   </td>
   </tr>
   
   <tr align="center" class="smallneutral">
   <td>
      Copyright &#169; 2013, Trustonic Limited. All rights reserved
      <br/>
      This is a <b>Confidential Document</b>
   </td>
   </tr>
   </table>
</xsl:template> 

<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
<!-- #######                           DISPLAY FUNCTIONS                                ####### -->
<!-- ########################################################################################## -->
<!-- ########################################################################################## -->

<!-- ########################################################################################## -->
<!-- #######                     Convert the time in readable format                    ####### -->
<!-- ########################################################################################## -->
<xsl:template name="display_time">
   <xsl:param name="mseconds"/>
   
    <xsl:variable name="hh" select="floor($mseconds div 3600000)"/>
    <xsl:variable name="mm" select="floor($mseconds div 60000) mod 60"/>
    <xsl:variable name="ss" select="floor($mseconds div 1000) mod 60"/>
    <xsl:variable name="cs" select="floor($mseconds div 10) mod 100"/>

    <xsl:if test="$hh &gt; 0">
        <xsl:value-of select="$hh"/>
        <xsl:text>h</xsl:text>
    </xsl:if>
    
    <xsl:if test="$mm &gt; 0">
        <xsl:value-of select="$mm"/>
        <xsl:text>m</xsl:text>
    </xsl:if>
    
    <xsl:if test="$hh = 0">
       <xsl:value-of select="$ss"/>
       
       <xsl:if test="$mm = 0 and $cs &gt; 0">
           <xsl:text>.</xsl:text>
           <xsl:value-of select="format-number($cs, '00')"/>
       </xsl:if>
       
       <xsl:text>s</xsl:text>
    </xsl:if>
    
</xsl:template>

<!-- ########################################################################################## -->
<!-- #######            Display target, product package and test package info           ####### -->
<!-- ########################################################################################## -->
<xsl:template name="display_configuration">
   <xsl:param name="targetName" />
   <xsl:param name="productPackageName" />
   <xsl:param name="testPackageName" />
   <xsl:param name="testSuiteName" />
   <xsl:param name="binaries_version" />
   <xsl:param name="date" />
   <xsl:param name="isCompare" />
   
   <td class="tableHeader" align="left">
      <xsl:attribute name="colspan">
         <xsl:value-of select="2+number(/TestReports/display/@execution_time='True')-2*number($isCompare='True')"/>
      </xsl:attribute>
   
      <div align="left">Product version:</div>
      <div align="right"> <xsl:value-of select="$productPackageName"/></div>
      <br/>
      
      <div align="left">Test Suite:</div>
      <div align="right"> <xsl:value-of select="$testSuiteName"/></div>
      <br/>
      
      <div align="left">Test version:</div>
      <div align="right"> <xsl:value-of select="$testPackageName"/></div>
      <br/>
      
      <div align="left">Platform:</div>
      <div align="right"> <xsl:value-of select="$targetName"/></div>
      <br/>
      
      <div align="left">Binaries version:</div>
      <div align="right"> <xsl:value-of select="$binaries_version"/></div>
      <br/>
      
      <div align="left">Execution date:</div>
      <div align="right"> <xsl:value-of select="$date"/></div>
      <br/>
      
   </td>
   

</xsl:template>

<!-- ########################################################################################## -->
<!-- #######                        Display link to failure page                        ####### -->
<!-- ########################################################################################## -->
<xsl:template name="display_failure_link">
   <td align="left">
      <a>
         <xsl:attribute name="href">
            <xsl:value-of select="concat($htmlPageName,'?page=failures')" />
         </xsl:attribute>
         Link to all failures
      </a>
   </td>
</xsl:template>

<!-- ########################################################################################## -->
<!-- #######                       Display link to previous pages                       ####### -->
<!-- ########################################################################################## -->
<xsl:template name="display_links">
   <xsl:param name="isBackToIndex"/>
   <xsl:param name="BackToGoup"/>

   <td colspan="100"> <!-- force to span to the max-->
      <table width="100%">
         <td align="left">
            <xsl:if test="$BackToGoup != ''">
               <a>
                  <xsl:attribute name="href">
                     <xsl:value-of select="concat($htmlPageName,'?group=', $BackToGoup)" />
                  </xsl:attribute>
                  Back to group report
               </a>
            </xsl:if>
         </td>
         
         <td></td>
         
         <td align="right">
            <xsl:if test="$isBackToIndex = 'True'">
               <a>
                  <xsl:attribute name="href">
                     <xsl:value-of select="$htmlPageName" />
                  </xsl:attribute>
                  Back to index
               </a>
            </xsl:if>
         </td>
      </table>
   </td>
</xsl:template>

<!-- ########################################################################################## -->
<!-- #######                 Display the time value in a readable format                ####### -->
<!-- ########################################################################################## -->
<xsl:template name="display_time_total">
   <xsl:param name="sum_time_total"/>

   <!-- total test time -->
   <xsl:if test="$sum_time_total >0">
      <xsl:call-template name="display_time">
         <xsl:with-param name="mseconds" select="round($sum_time_total)"/>
      </xsl:call-template>
   </xsl:if>
</xsl:template>

<!-- ########################################################################################## -->
<!-- #######             Display the mini numerical status of a test group              ####### -->
<!-- ########################################################################################## -->
<xsl:template name="display_group_total">
   <xsl:param name="totalTC"/>
   <xsl:param name="totalTCSuccess"/>
   <xsl:param name="totalTCFailure" />
   <xsl:param name="totalTCKnownFailure"/>
   <xsl:param name="totalTCWarning"/>
   <xsl:param name="totalTCNotExecuted"/>
   <xsl:param name="totalTCUndocumented"/>
   <xsl:param name="nbCrash"/>
   <xsl:param name="nbTimeout"/>
      
   <xsl:if test="$totalTC != 0">
      <!-- TOTAL -->
      <div class="smallNeutral" align="center">
         <xsl:value-of select="$totalTC"/>
         <xsl:if test="not ($totalTCNotExecuted = '0')">
            <br/><xsl:value-of select="$totalTCNotExecuted"/> not executed
         </xsl:if>
      </div>

      <!-- SUCCESSES -->
      <div class="smallGreen" align="center">
         <xsl:value-of select="$totalTCSuccess + $totalTCWarning"/>
         <xsl:if test="not ($totalTCWarning = '0')">
            &#160;(<xsl:value-of select="$totalTCWarning"/> warnings)
         </xsl:if>
      </div>

      <!-- FAILURES -->
      <div class="smallRed" align="center">
         <xsl:value-of select="$totalTCKnownFailure + $totalTCFailure"/>
         <xsl:if test="not ($totalTCKnownFailure = '0')">
            &#160;(<xsl:value-of select="$totalTCKnownFailure"/> documented)
         </xsl:if>
      </div>
      
      <!-- UNDOCUMENTED -->
      <div class="smallOrange" align="center">
         <xsl:if test="not ($totalTCUndocumented = '0')">
            Warning! <xsl:value-of select="$totalTCUndocumented"/> test case(s) not in test plan
         </xsl:if>
      </div>

   </xsl:if>
   
   <xsl:if test="$nbCrash != 0">
      <div class="smallBoldRed">CRASH (<xsl:value-of select="$nbCrash"/>)</div>
   </xsl:if>
   <xsl:if test="$nbTimeout != 0">
      <div class="smallBoldOrange">TIMEOUT (<xsl:value-of select="$nbTimeout"/>)</div>
   </xsl:if>
   
</xsl:template>

<!-- ########################################################################################## -->
<!-- #######                Display the mini numerical status of a test                 ####### -->
<!-- ########################################################################################## -->
<xsl:template name="display_test_total">
   <xsl:param name="totalTC"/>
   <xsl:param name="totalTCNotExecuted"/>
   <xsl:param name="totalTCSuccess"/>
   <xsl:param name="totalTCFailure"/>
   <xsl:param name="totalTCWarning"/>
   <xsl:param name="groupName"/>
   <xsl:param name="testName"/>
   <xsl:param name="targetName"/>
   <xsl:param name="totalTCKnownFailure"/>
   <xsl:param name="totalTCUndocumented"/>
   <xsl:param name="status"/>
   <xsl:param name="testBinaryName"/>
   <xsl:param name="isBlacklisted"/>
   <xsl:param name="res_path"/>

   <xsl:variable name="binaryExt">
      <xsl:if test="$isBlacklisted='true'">_BlackList</xsl:if>
   </xsl:variable>

      <xsl:choose>
         <xsl:when test="$totalTC =0">
            <br/>
         </xsl:when>
         <xsl:when test="$totalTC = $totalTCSuccess + $totalTCWarning + $totalTCKnownFailure">
            <div class="smallGreen"><img src="conf/greenbullet.gif"/> pass</div>
         </xsl:when>
         <xsl:otherwise>
            <div class="smallRed"><img src="conf/redbullet.gif"/> fail</div>
         </xsl:otherwise>
      </xsl:choose>

      <xsl:if test="$totalTC - $totalTCNotExecuted &gt; 0">
         <a class="smallLink" target="popup">
            <xsl:attribute name="href">
               <xsl:value-of select="concat($res_path,'/reports_' , $testBinaryName , $binaryExt , '/', $testName, '_user.txt')" />
            </xsl:attribute>
            usr
         </a>
         <a class="smallLink" target="popup">
            <xsl:attribute name="href">
               <xsl:value-of select="concat($res_path,'/reports_' , $testBinaryName , $binaryExt , '/', $testName, '_kernel.txt')" />
            </xsl:attribute>
            kern
         </a>
         <a class="smallLink" target="popup">
            <xsl:attribute name="href">
               <xsl:value-of select="concat($res_path,'/reports_' , $testBinaryName , $binaryExt , '/', $testName, '_debug.txt')" />
            </xsl:attribute>
            dbg
         </a>
      </xsl:if>
      
      <xsl:if test="$status">
         <xsl:if test="$status='CRASH'">
            <div class="smallBoldRed">CRASH</div>
         </xsl:if>
         <xsl:if test="$status='TIMEOUT'">
            <div class="smallBoldOrange">TIMEOUT</div>
         </xsl:if>
      </xsl:if>
      
      <!-- TOTAL -->
      <div class="smallNeutral">
         <xsl:value-of select="$totalTC"/>
         <xsl:if test="$totalTCNotExecuted &gt; 0">
            <br/><xsl:value-of select="$totalTCNotExecuted"/> not executed
         </xsl:if>
      </div>

      <!-- SUCCESSES -->
      <div class="smallGreen">
         <xsl:value-of  select="$totalTCSuccess + $totalTCWarning"/>
         <xsl:if test="$totalTCWarning &gt; 0">
            &#160;(<xsl:value-of select="$totalTCWarning"/> warnings)
      </xsl:if>
      </div>

      <!-- FAILURES -->
      <div class="smallRed">
         <xsl:value-of select="$totalTCFailure + $totalTCKnownFailure"/>
         <xsl:if test="$totalTCKnownFailure &gt; 0">
            &#160;(<xsl:value-of select="$totalTCKnownFailure"/> documented)
         </xsl:if>
      </div>
   
      <!-- UNDOCUMENTED -->
      <div class="smallOrange">
         <xsl:if test="$totalTCUndocumented &gt; 0">
            Warning! <xsl:value-of select="$totalTCUndocumented"/> test case(s) not in test plan
         </xsl:if>
      </div>
</xsl:template>


<!-- ########################################################################################## -->
<!-- #######              Display the mini numerical status of a test case              ####### -->
<!-- ########################################################################################## -->
<xsl:template name="display_test_case_total">
   <xsl:param name="testcaseResult"/>
   <xsl:param name="testcaseDoc"/>
   <xsl:param name="testcaseMsg"/>
   <xsl:param name="testcaseBugMsg"/>
   <xsl:param name="testcaseBugLink"/>

   <xsl:if test="$testcaseResult = 'SUCCESS'">
      <div class="smallGreen">
         <b>pass</b>
      </div>
   </xsl:if>
   <xsl:if test="$testcaseResult = 'FAILURE'">
      <div class="smallRed">
         <b>fail</b>
         <br/>
      </div>
      <div align="left">
         <xsl:copy-of select="$testcaseMsg"/>
      </div>
   </xsl:if>
   <xsl:if test="$testcaseResult = 'KNOWN_FAILURE'"> 
      <div class="smallViolet">
         <b>Documented Failure</b>
         
         <xsl:if test="$testcaseBugLink != ''"> 
            <br/>
            
            <xsl:choose>
            <xsl:when test="/TestReports/display/@jira_link='True'">
               <a class="smallLink" target="popup">
                  <xsl:attribute name="href">
                     <xsl:value-of select="concat($linkJira, $testcaseBugLink)" />
                  </xsl:attribute>
                  Issue ID: <xsl:value-of select="$testcaseBugLink"/>
               </a>
            </xsl:when>
            <xsl:otherwise>
               Issue ID: <xsl:value-of select="$testcaseBugLink"/>
            </xsl:otherwise>
            </xsl:choose> 
            
         </xsl:if>
      </div>
      <xsl:if test="$testcaseBugMsg != ''"> 
         <div>
            <xsl:value-of select="$testcaseBugMsg"/>
         </div>
      </xsl:if>
   </xsl:if>
   <xsl:if test="$testcaseResult = 'SHOULD_FAIL_BUT_SUCCESS'">
      <div class="smallOrange">
         <b>Warning</b>
         <br/>
         Passed but described as failure in the conformance file:
         <xsl:if test="$testcaseBugMsg != ''"> 
            <div align="left">
               <xsl:value-of select="$testcaseBugMsg"/>
            </div>
         </xsl:if>
      </div>
   </xsl:if>
   <xsl:if test="$testcaseResult = 'NOT_EXECUTED'">
      <img src="conf/failed.gif"/>
      <br/>
   </xsl:if>
      
   <!-- UNDOCUMENTED -->
   <xsl:if test="$testcaseDoc = 'NOT_DOCUMENTED'">
      <div class="smallOrange">
      <b>Warning! this test case is not in the test plan</b>
      </div>
   </xsl:if>
</xsl:template>


<!-- ########################################################################################## -->
<!-- #######                       Display documentation link                           ####### -->
<!-- ########################################################################################## -->
<xsl:template name="display_doc_link">
   <xsl:param name="blacklistLink"/>
   <xsl:param name="docLink"/>
   <xsl:param name="isdocDocumented"/>
   <xsl:param name="isBlacklistDocumented"/>
   <xsl:param name="displayDoc"/>
   <xsl:param name="displayBlacklist"/>

   <xsl:if test="$displayDoc != ''">
      <xsl:choose>
		<xsl:when test="$isdocDocumented != ''">
         <a class="smallLink" target="popup">
            <xsl:attribute name="href">
               <xsl:value-of select="$docLink" />
            </xsl:attribute>
            <xsl:attribute name="target">
               basefrm
            </xsl:attribute>
            <img src="conf/doc.gif"/>
         </a>
		</xsl:when>
		<xsl:otherwise>
         <img src="conf/nodoc.gif"/>
		</xsl:otherwise>
	   </xsl:choose>
   </xsl:if>
      
   <xsl:if test="$displayBlacklist != ''">
      <xsl:choose>
		<xsl:when test="$isBlacklistDocumented != ''">
         <a class="smallLink" target="popup">
            <xsl:attribute name="href">
               <xsl:value-of select="$blacklistLink" />
            </xsl:attribute>
            <xsl:attribute name="target">
               basefrm
            </xsl:attribute>
            <img src="conf/blacklist.gif"/>
         </a>
		</xsl:when>
		<xsl:otherwise>
         <img src="conf/noblacklist.gif"/>
		</xsl:otherwise>
	   </xsl:choose>
   </xsl:if>
      

</xsl:template> 

<!-- ########################################################################################## -->
<!-- #######                        Display failures informations                       ####### -->
<!-- ########################################################################################## -->
<xsl:template name="display_failures_info">
   <xsl:param name="displayName"/>
   <xsl:param name="failureType"/>
   <xsl:param name="severity"/>
   
   <xsl:variable name="nbFail">
   <xsl:choose>
      <xsl:when test="$failureType = 'KNOWN_FAILURE'">
         <xsl:value-of select="count(./group/test/test_case[@result=$failureType and @severity=$severity])"/>
      </xsl:when>
      <xsl:otherwise>
         <xsl:value-of select="count(./group/test/test_case[@result=$failureType or @documented=$failureType])"/>
      </xsl:otherwise>
   </xsl:choose>
   </xsl:variable>
 
 
   <xsl:if test="$nbFail != 0">
      <tr>
         <td  colspan='5'>         
            <h3 align='left'><xsl:value-of select="$displayName"/>:</h3>
         </td>
      </tr>
      
      <tr class="tableHeader">
         <td>
            <b>Group</b>
         </td>
         <td>
            <b>Test</b>
         </td>
         <td>
            <b>Test case</b>
         </td>
         <td>
            <b>Doc.</b>
         </td>
         <xsl:if test="$failureType = 'FAILURE'">
         <td colspan='2'>
            <b>Message</b>
         </td>
         </xsl:if>
         <xsl:if test="$failureType = 'KNOWN_FAILURE' or $failureType = 'SHOULD_FAIL_BUT_SUCCESS'">
         <td>
            <b>Issue ID</b>
         </td>
         <td>
            <b>Conformance message</b>
         </td>
         </xsl:if>
         
      </tr>
   </xsl:if>

      <xsl:for-each select="./group/test/test_case[@result=$failureType or @documented=$failureType]">
         <xsl:variable name="testcaseName" select="@name"/>
         <xsl:variable name="testName" select="../@name"/>
         <xsl:variable name="groupName" select="../../@name"/>
         
         <xsl:if test="$failureType != 'KNOWN_FAILURE' or @severity=$severity">
            <tr>
               <td class="tableDescr">
                  <b><xsl:value-of select="$groupName"/></b>
               </td>
               <td class="tableDescr">
                  <b><xsl:value-of select="$testName"/></b>
               </td>
               <td class="tableContent">
                  <b><xsl:value-of select="$testcaseName"/></b>
               </td>
               <td class="tableContent">
                  <xsl:call-template name="display_doc_link">
                     <xsl:with-param name="blacklistLink" select="concat(../../../@res_path, '/doc/', ../../../@test_suite_name , '_BlackList.html/', ../@doc_ref, '.html#', @doc_ref)"/>
                     <xsl:with-param name="docLink" select="concat(../../../@res_path, '/doc/', ../../../@test_suite_name , '.html/', ../@doc_ref, '.html#', @doc_ref)"/>
                     <xsl:with-param name="isdocDocumented">
                        <xsl:if test="@documented = 'DOCUMENTED'">
                           Ok
                        </xsl:if>
                     </xsl:with-param>
                     <xsl:with-param name="isBlacklistDocumented">
                        <xsl:if test="@documented = 'DOCUMENTED'">
                           Ok
                        </xsl:if>
                     </xsl:with-param>
                     <xsl:with-param name="displayBlacklist">
                        <xsl:if test="../@blacklisted = 'true'">
                           Ok
                        </xsl:if>
                     </xsl:with-param>
                     <xsl:with-param name="displayDoc">
                        <xsl:if test="not(../@blacklisted)">
                           Ok
                        </xsl:if>
                     </xsl:with-param>
                  </xsl:call-template>
               </td>
               <xsl:if test="$failureType = 'FAILURE'">
               <td class="tableContent" colspan='2'>
                  <xsl:copy-of select="./message/."/>
               </td>
               </xsl:if>
               <xsl:if test="$failureType = 'KNOWN_FAILURE' or $failureType = 'SHOULD_FAIL_BUT_SUCCESS'">
               <td class="tableContent">
                  <xsl:choose>
                  <xsl:when test="/TestReports/display/@jira_link='True'">
                     <a class="smallLink" target="popup">
                        <xsl:attribute name="href">
                           <xsl:value-of select="concat($linkJira, @issue_ID)" />
                        </xsl:attribute>
                        <xsl:value-of select="@issue_ID"/>
                     </a>
                  </xsl:when>
                  <xsl:otherwise>
                     <xsl:value-of select="@issue_ID"/>
                  </xsl:otherwise>
                  </xsl:choose> 
            
               </td>
               <td class="tableContent">
                  <xsl:value-of select="./description/."/>
               </td>
               </xsl:if>

<!--            <td class="tableContent" align="center">-->
<!--               <xsl:call-template name="display_test_case_total">-->
<!--                  <xsl:with-param name="testcaseResult" select="@result"/>-->
<!--                  <xsl:with-param name="testcaseDoc" select="@documented"/>-->
<!--               </xsl:call-template>-->
<!--            </td>-->
            </tr>
         </xsl:if>
      </xsl:for-each> 
</xsl:template>
       
<!-- ########################################################################################## -->
<!-- #######                        Display result in color                             ####### -->
<!-- ########################################################################################## -->
<xsl:template name="display_result">
   <xsl:param name="value"/>
   <xsl:param name="status"/>
   <b>
      <xsl:choose>
      <xsl:when test="$value='SUCCESS'">
         <span class="smallGreen"><xsl:value-of select="$value"/></span>
      </xsl:when>
      <xsl:when test="$value='FAILURE'">
         <span class="smallRed"><xsl:value-of select="$value"/></span>
      </xsl:when>
      <xsl:when test="$value='SHOULD_FAIL_BUT_SUCCESS'">
         <span class="smallOrange"><xsl:value-of select="$value"/></span>
      </xsl:when>
      <xsl:when test="$value='KNOWN_FAILURE'">
         <span class="smallViolet"><xsl:value-of select="$value"/></span>
      </xsl:when>
      <xsl:when test="$value='NOT_EXECUTED'">
         <span class="smallNeutral"><xsl:value-of select="$value"/></span>
      </xsl:when>
      <xsl:otherwise>
         <span class="smallNeutral">X</span>
      </xsl:otherwise>
      </xsl:choose>
      
      <xsl:choose>
      <xsl:when test="$status='TIMEOUT'">
         <span class="smallNeutral"> (</span>
         <span class="smallOrange"><xsl:value-of select="$status"/></span>
         <span class="smallNeutral">)</span>
      </xsl:when>
      <xsl:when test="$status='CRASH'">
         <span class="smallNeutral"> (</span>
         <span class="smallRed"><xsl:value-of select="$status"/></span>
         <span class="smallNeutral">)</span>
      </xsl:when>
      </xsl:choose>
   </b>      
</xsl:template> 

<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
<!-- #######                             COMPARE TABLE                                  ####### -->
<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
 <xsl:key name="TC" match="test_case" use="concat(../../@name,'.',../@name,'.',@name)"/>
 
<xsl:template name="display_compare_table">
   <table class="tableContainer">
  
  
   <tr>
      <td class="tableHeader"/>
         <xsl:for-each select="/TestReports/test_list">
            <xsl:call-template name="display_configuration">
               <xsl:with-param name="targetName" select="@platform"/>
               <xsl:with-param name="productPackageName" select="@tbase_version"/>
               <xsl:with-param name="testPackageName" select="@test_package_version"/>
               <xsl:with-param name="testSuiteName" select="@test_suite_name"/>
               <xsl:with-param name="binaries_version" select="@binaries_version"/>
               <xsl:with-param name="date" select="@date"/>
               <xsl:with-param name="isCompare" select="'True'"/>
            </xsl:call-template>
         </xsl:for-each>
   </tr>
   
   <tr>  
      <td class="tableDescr">
         Total metrics
      </td>
            
      <xsl:for-each select="/TestReports/test_list">
         
      <td class="tableSummarize">
         <xsl:call-template name="display_group_total">
            <xsl:with-param name="totalTC" select="count(./group/test/test_case)"/>
            <xsl:with-param name="totalTCSuccess" select="count(./group/test/test_case[@result='SUCCESS'])"/>
            <xsl:with-param name="totalTCFailure" select="count(./group/test/test_case[@result='FAILURE'])"/>
            <xsl:with-param name="totalTCKnownFailure" select="count(./group/test/test_case[@result='KNOWN_FAILURE'])"/>
            <xsl:with-param name="totalTCWarning" select="count(./group/test/test_case[@result='SHOULD_FAIL_BUT_SUCCESS'])"/>
            <xsl:with-param name="totalTCNotExecuted" select="count(./group/test/test_case[@result='NOT_EXECUTED'])"/>
            <xsl:with-param name="totalTCUndocumented" select="0"/> <!--don't display document failures...-->
            <xsl:with-param name="nbCrash" select="count(./group/test[@status='CRASH'])"/>
            <xsl:with-param name="nbTimeout" select="count(./group/test[@status='TIMEOUT'])"/>
         </xsl:call-template>
      </td>
      </xsl:for-each>              
   </tr>


<!--  <xsl:for-each select="/TestReports/test_list/group[not(preceding::group/@name = @name)]">-->
<!--      <xsl:variable name="groupName" select="@name"/>-->

<!--      <xsl:for-each select="/TestReports/test_list/group[@name=$groupName]/test[not(preceding::group[@name=$groupName]/test/@name = @name)]">-->
<!--         <xsl:variable name="testName" select="@name"/>-->

<!--         <xsl:for-each select="/TestReports/test_list/group[@name=$groupName]/test[@name=$testName]/test_case[not(preceding::group[@name=$groupName]/test[@name=$testName]/test_case/@name = @name)]">-->
<!--            <xsl:variable name="testcaseName" select="@name"/>-->


<!--               <xsl:value-of select="$groupName"/>.<xsl:value-of select="$testName"/>.<xsl:value-of select="$testcaseName"/><br/>-->
<!--         </xsl:for-each>   -->
<!--      </xsl:for-each>  -->
<!--   </xsl:for-each>   -->
   
   	
	
	
<!--	[count(. | key('TC', ../../@name|'.'|../@name|'.'|@name)[1]) = 1]-->
	
<!-- <xsl:for-each select="/TestReports/test_list/group/test/test_case[count(. | key('TC', concat(../../@name,'.',../@name,'.',@name))[1]) = 1]">-->
<!--	<xsl:sort select="../../@name" order="ascending"/>-->
<!--	<xsl:sort select="../@name" order="ascending"/>-->
<!--	<xsl:sort select="@name" order="ascending"/>-->
<!--   <xsl:variable name="groupName" select="../../@name"/>-->
<!--   <xsl:variable name="testName" select="../@name"/>-->
<!--   <xsl:variable name="testcaseName" select="@name"/>-->
<!--	-->
<!--	<xsl:for-each select="/TestReports/test_list">-->
<!--	-->
<!--   <xsl:value-of select="$groupName"/>.<xsl:value-of select="$testName"/>.<xsl:value-of select="$testcaseName"/><br/>-->

<!--</xsl:for-each> -->
	






<!--<xsl:key name="contacts-by-surname" match="contact" use="surname" />-->

<!--<xsl:for-each select="contact[count(. | key('contacts-by-surname', surname)[1]) = 1]">-->
<!--		<xsl:sort select="surname" />-->
<!--		<xsl:value-of select="surname" />,<br />-->
<!--		<xsl:for-each select="key('contacts-by-surname', surname)">-->
<!--			<xsl:sort select="forename" />-->
<!--			<xsl:value-of select="forename" /> (<xsl:value-of select="title" />)<br />-->
<!--		</xsl:for-each>-->
<!--	</xsl:for-each>-->
	
	
	
	
   	
   
<!--         <xsl:for-each select="/TestReports/test_list/group/test/test_case">-->
<!--         -->
<!--         	<xsl:sort select="../../@name" order="ascending"/>-->
<!--         	<xsl:sort select="../@name" order="ascending"/>-->
<!--         	<xsl:sort select="@name" order="ascending"/>-->
<!--      	-->
<!--      	-->
<!--            <xsl:variable name="groupName" select="../../@name"/>-->
<!--            <xsl:variable name="testName" select="../@name"/>-->
<!--            <xsl:variable name="testcaseName" select="@name"/>-->
<!--.-->

<!--            <xsl:if test="../../../group[@name=$groupName]/test[@name=$testName]/test_case[@name=$testcaseName]/@result != /TestReports/test_list[1]/group[@name=$groupName]/test[@name=$testName]/test_case[@name=$testcaseName]/@result">-->

<!--               <xsl:value-of select="$groupName"/>.<xsl:value-of select="$testName"/>.<xsl:value-of select="$testcaseName"/><br/>-->
<!--            </xsl:if>-->
<!--         </xsl:for-each>   -->






<!--  <xsl:for-each select="/TestReports/test_list/group">-->
<!--      <xsl:variable name="groupName" select="@name"/>-->

<!--      <xsl:for-each select="./test">-->
<!--         <xsl:variable name="testName" select="@name"/>-->

<!--         <xsl:for-each select="./test_case[count(. | key('TC', concat(../../@name,'.',../@name,'.',@name))[1]) = 1]">-->
<!--            <xsl:variable name="testcaseName" select="@name"/>-->


<!--               <xsl:value-of select="$groupName"/>.<xsl:value-of select="$testName"/>.<xsl:value-of select="$testcaseName"/><br/>-->
<!--         </xsl:for-each>   -->
<!--      </xsl:for-each>  -->
<!--   </xsl:for-each>   -->




      <xsl:for-each select="/TestReports/test_list/group">
         <xsl:variable name="groupName" select="@name"/>
            
<xsl:variable name="diff_group">
         <xsl:for-each select="./test">
            <xsl:variable name="testName" select="@name"/>
         
         
<xsl:variable name="diff_test">
            <xsl:for-each select="./test_case[count(. | key('TC', concat(../../@name,'.',../@name,'.',@name))[1]) = 1]">
         <xsl:variable name="testcaseName" select="@name"/>
     
     
     
     <xsl:variable name="diff_testcase">
     <xsl:variable name="ref" select="(key('TC', concat(../../@name,'.',../@name,'.',@name))[1])/@result"/>
         
       <xsl:if test="count(key('TC', concat(../../@name,'.',../@name,'.',@name))) != count(/TestReports/test_list)">
          nok
       </xsl:if>
         
       <xsl:for-each select="key('TC', concat(../../@name,'.',../@name,'.',@name))">
            <xsl:if test="./@result != $ref">
               nok
            </xsl:if>
       </xsl:for-each>   
    </xsl:variable>
     
     
     <xsl:if test="$diff_testcase != ''">
     <tr>
         <td class="tableContent">
            <xsl:value-of select="$testcaseName"/>
         </td>
        <xsl:for-each select="/TestReports/test_list">
            <td class="tableContent" align="center">
               <xsl:call-template name="display_result">
                  <xsl:with-param name="value" select="./group[@name=$groupName]/test[@name=$testName]/test_case[@name=$testcaseName]/@result"/>
                  <xsl:with-param name="status" select="./group[@name=$groupName]/test[@name=$testName]/@status"/>
               </xsl:call-template>
            </td>
        </xsl:for-each> 
     </tr>
      
     </xsl:if>
     
     
            </xsl:for-each>   
              
</xsl:variable>
<xsl:if test="$diff_test != ''">
   <tr>
      <td class="tableDescr">
         <xsl:value-of select="$testName"/>
      </td>
     <xsl:for-each select="/TestReports/test_list">
         <td class="tableDescr"/>
     </xsl:for-each> 
   </tr> 
   <xsl:copy-of select="$diff_test"/>
</xsl:if>
     
         </xsl:for-each>  
           
</xsl:variable>
<xsl:if test="$diff_group != ''">
   <tr>
      <td class="tableHeader">
         <xsl:value-of select="$groupName"/>
      </td>
     <xsl:for-each select="/TestReports/test_list">
         <td class="tableHeader"/>
     </xsl:for-each> 
   </tr>
   <xsl:copy-of select="$diff_group"/>
</xsl:if>

      </xsl:for-each> 
  
  
   <tr>
      <xsl:call-template name="display_links">
         <xsl:with-param name="isBackToIndex" select="'True'"/>
         <xsl:with-param name="BackToGoup" select="''"/>
      </xsl:call-template>
   </tr>
  
   
   </table>
</xsl:template> 
<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
<!-- #######                            FAILURES REPORT                                 ####### -->
<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
<xsl:template name="display_failures_table">
  <table class="tableContainer">
   
   <xsl:if test="count(/TestReports/test_list) > 1">
      <tr>
         <td  colspan='6'>
            <h2 align='left'>
               <br/>Summary<br/>
            </h2>
         </td>
      </tr>
      <br/><br/>
       
      <xsl:for-each select="/TestReports/test_list">
         <xsl:if test="count(./group/test/test_case[@result != 'SUCCESS']) != 0">
         
       <tr>
       <td  colspan='6'>
            <xsl:variable name="linkval" select="concat(@tbase_version, @test_package_version, @test_suite_name, @platform, @binaries_version, @date)"/>
            
            <a>
               <xsl:attribute name="href">
               #<xsl:value-of select="$linkval"/>
               </xsl:attribute>
         
               Product version: <xsl:value-of select="@tbase_version"/><br/>
               Test Suite: <xsl:value-of select="@test_package_version"/><br/>
               Test version: <xsl:value-of select="@test_suite_name"/><br/>
               Platform: <xsl:value-of select="@platform"/><br/>
               Binaries version: <xsl:value-of select="@binaries_version"/><br/>
               Execution date: <xsl:value-of select="@date"/><br/>
               <br/><br/>
            </a>

       </td>
       </tr>
         </xsl:if>
      </xsl:for-each> 
   </xsl:if>
   
   <xsl:for-each select="/TestReports/test_list">
      <xsl:if test="count(./group/test/test_case[@result != 'SUCCESS']) != 0">
         <tr>
            <xsl:variable name="linkval" select="concat(@tbase_version, @test_package_version, @test_suite_name, @platform, @binaries_version, @date)"/>
            <xsl:attribute name="id">
               <xsl:value-of select="$linkval"/>
            </xsl:attribute>
            
            <td  colspan='6'>
               <h2 align='left'>
                  Product version: <xsl:value-of select="@tbase_version"/><br/>
                  Test Suite: <xsl:value-of select="@test_package_version"/><br/>
                  Test version: <xsl:value-of select="@test_suite_name"/><br/>
                  Platform: <xsl:value-of select="@platform"/><br/>
                  Binaries version: <xsl:value-of select="@binaries_version"/><br/>
                  Execution date: <xsl:value-of select="@date"/><br/>
               </h2>
            </td>
         </tr>
        
         <tr>
            <br/>
         </tr>
                       
         <xsl:call-template name="display_failures_info">
            <xsl:with-param name="displayName" select="'Unexpected failures'"/>
            <xsl:with-param name="failureType" select="'FAILURE'"/>
         </xsl:call-template>
                       
         <xsl:call-template name="display_failures_info">
            <xsl:with-param name="displayName" select="'Critical expected failures'"/>
            <xsl:with-param name="failureType" select="'KNOWN_FAILURE'"/>
            <xsl:with-param name="severity" select="'critical'"/>
         </xsl:call-template>
         
         <xsl:call-template name="display_failures_info">
            <xsl:with-param name="displayName" select="'Major expected failures'"/>
            <xsl:with-param name="failureType" select="'KNOWN_FAILURE'"/>
            <xsl:with-param name="severity" select="'major'"/>
         </xsl:call-template>
         
         <xsl:call-template name="display_failures_info">
            <xsl:with-param name="displayName" select="'Minor expected failures'"/>
            <xsl:with-param name="failureType" select="'KNOWN_FAILURE'"/>
            <xsl:with-param name="severity" select="'minor'"/>
         </xsl:call-template>
             
         <xsl:call-template name="display_failures_info">
            <xsl:with-param name="displayName" select="'Should failed but success'"/>
            <xsl:with-param name="failureType" select="'SHOULD_FAIL_BUT_SUCCESS'"/>
         </xsl:call-template>
                   
         <xsl:call-template name="display_failures_info">
            <xsl:with-param name="displayName" select="'Not in test plan'"/>
            <xsl:with-param name="failureType" select="'NOT_DOCUMENTED'"/>
         </xsl:call-template>
         
         <xsl:call-template name="display_failures_info">
            <xsl:with-param name="displayName" select="'Not executed'"/>
            <xsl:with-param name="failureType" select="'NOT_EXECUTED'"/>
         </xsl:call-template>
         
      </xsl:if>
   </xsl:for-each> 
   <tr>
      <xsl:call-template name="display_links">
         <xsl:with-param name="isBackToIndex" select="'True'"/>
         <xsl:with-param name="BackToGoup" select="''"/>
      </xsl:call-template>
   </tr>
   </table>

  
</xsl:template>


<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
<!-- #######                              GROUP REPORT                                  ####### -->
<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
<xsl:template name="display_group_table">

      <table class="tableContainer">
         <tr align="center">
            <td class="tableHeader" rowspan="2">Test</td>
            
         <xsl:for-each select="/TestReports/test_list">
               <xsl:call-template name="display_configuration">
                  <xsl:with-param name="targetName" select="@platform"/>
                  <xsl:with-param name="productPackageName" select="@tbase_version"/>
                  <xsl:with-param name="testPackageName" select="@test_package_version"/>
                  <xsl:with-param name="testSuiteName" select="@test_suite_name"/>
                  <xsl:with-param name="binaries_version" select="@binaries_version"/>
                  <xsl:with-param name="date" select="@date"/>
               </xsl:call-template>
            </xsl:for-each>
         </tr>
         
         <tr align="center" class="tableHeader">
            <xsl:for-each select="/TestReports/test_list">
               <td>
                  Results
               </td>   
               <td>
                  Doc.
               </td>
                  
               <!-- if add_exec_time was sent as parameter, show this column -->
               <xsl:if test="/TestReports/display/@execution_time='True'">
                  <td>
                     Time (s)
                  </td>
               </xsl:if>
            </xsl:for-each>
         </tr>

         <!-- The row that shows the total metrics -->
         <tr align="center">
            <td class="tableDescr">
               Total metrics
            </td>

            <xsl:for-each select="/TestReports/test_list">
                  
               <td class="tableSummarize">
                  <xsl:call-template name="display_group_total">
                     <xsl:with-param name="totalTC" select="count(./group/test/test_case)"/>
                     <xsl:with-param name="totalTCSuccess" select="count(./group/test/test_case[@result='SUCCESS'])"/>
                     <xsl:with-param name="totalTCFailure" select="count(./group/test/test_case[@result='FAILURE'])"/>
                     <xsl:with-param name="totalTCKnownFailure" select="count(./group/test/test_case[@result='KNOWN_FAILURE'])"/>
                     <xsl:with-param name="totalTCWarning" select="count(./group/test/test_case[@result='SHOULD_FAIL_BUT_SUCCESS'])"/>
                     <xsl:with-param name="totalTCNotExecuted" select="count(./group/test/test_case[@result='NOT_EXECUTED'])"/>
                     <xsl:with-param name="totalTCUndocumented" select="count(./group/test/test_case[@documented='NOT_DOCUMENTED'])"/>
                     <xsl:with-param name="nbCrash" select="count(./group/test[@status='CRASH'])"/>
                     <xsl:with-param name="nbTimeout" select="count(./group/test[@status='TIMEOUT'])"/>
                  </xsl:call-template>
               </td>
               <td class="tableSummarize"/>
          
               <xsl:if test="/TestReports/display/@execution_time='True'">
                  <td class="tableSummarize">
                     <xsl:call-template name="display_time_total">
                        <xsl:with-param name="sum_time_total" select="sum(./group/test/@time_in_ms)"/>
                     </xsl:call-template>
                  </td>
               </xsl:if>
            </xsl:for-each>
            
         </tr>

         <xsl:for-each select="/TestReports/test_list/group[not(preceding::group/@name = @name)]">
            <xsl:variable name="groupName" select="@name" />

            <tr align="center">
               <td class="tableDescr">
                  <a>
                     <xsl:attribute name="href">
                        <xsl:value-of select="concat($htmlPageName,'?group=',$groupName)" />
                     </xsl:attribute>
                     <xsl:value-of select="$groupName"/>
                  </a>
               </td>

               <xsl:for-each select="/TestReports/test_list">               
                  <td class="tableContent">
                     <xsl:call-template name="display_group_total">
                        <xsl:with-param name="totalTC" select="count(./group[@name=$groupName]/test/test_case)"/>
                        <xsl:with-param name="totalTCSuccess" select="count(./group[@name=$groupName]/test/test_case[@result='SUCCESS'])"/>
                        <xsl:with-param name="totalTCFailure" select="count(./group[@name=$groupName]/test/test_case[@result='FAILURE'])"/>
                        <xsl:with-param name="totalTCKnownFailure" select="count(./group[@name=$groupName]/test/test_case[@result='KNOWN_FAILURE'])"/>
                        <xsl:with-param name="totalTCWarning" select="count(./group[@name=$groupName]/test/test_case[@result='SHOULD_FAIL_BUT_SUCCESS'])"/>
                        <xsl:with-param name="totalTCNotExecuted" select="count(./group[@name=$groupName]/test/test_case[@result='NOT_EXECUTED'])"/>
                        <xsl:with-param name="totalTCUndocumented" select="count(./group[@name=$groupName]/test/test_case[@documented='NOT_DOCUMENTED'])"/>
                        <xsl:with-param name="nbCrash" select="count(./group[@name=$groupName]/test[@status='CRASH'])"/>
                        <xsl:with-param name="nbTimeout" select="count(./group[@name=$groupName]/test[@status='TIMEOUT'])"/>
                     </xsl:call-template>
                  </td>
                  
                  
                  <xsl:variable name="totalTCBlacklist" select="count(./group[@name=$groupName]/test[@blacklisted]/test_case)"/>
                  <xsl:variable name="totalTCBDoc" select="count(./group[@name=$groupName]/test[not(@blacklisted)]/test_case)"/>
                  
                  <td class="tableContent" align="center">
                     <xsl:call-template name="display_doc_link">
                        <xsl:with-param name="blacklistLink" select="concat(@res_path, '/doc/', @test_suite_name , '_BlackList.html/', ./group[@name=$groupName]/@doc_ref, '.html')"/>
                        <xsl:with-param name="docLink" select="concat(@res_path, '/doc/', @test_suite_name , '.html/', ./group[@name=$groupName]/@doc_ref, '.html')"/>
                        <xsl:with-param name="isdocDocumented">
                           <xsl:if test="./group[@name=$groupName]/@doc_ref">
                              Ok
                           </xsl:if>
                        </xsl:with-param>
                        <xsl:with-param name="isBlacklistDocumented">
                           <xsl:if test="./group[@name=$groupName]/@doc_ref">
                              Ok
                           </xsl:if>
                        </xsl:with-param>
                        <xsl:with-param name="displayBlacklist">
                           <xsl:if test="$totalTCBlacklist != 0">
                              Ok
                           </xsl:if>
                        </xsl:with-param>
                        <xsl:with-param name="displayDoc">
                           <xsl:if test="$totalTCBDoc != 0">
                              Ok
                           </xsl:if>
                        </xsl:with-param>
                     </xsl:call-template>
                  </td>
                  
                  
                  
                  
                  
                  <xsl:if test="/TestReports/display/@execution_time='True'">
                     <td class="tableContent">
                        <xsl:call-template name="display_time_total">
                           <xsl:with-param name="sum_time_total" select="sum(./group[@name=$groupName]/test/@time_in_ms)"/>
                        </xsl:call-template>
                     </td>
                  </xsl:if>            
               </xsl:for-each>
            </tr>
         </xsl:for-each>

         <tr>
            <xsl:call-template name="display_links">
               <xsl:with-param name="isBackToIndex" select="'False'"/>
               <xsl:with-param name="BackToGoup" select="''"/>
            </xsl:call-template>
         </tr>
      </table>
</xsl:template>

<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
<!-- #######                               TEST REPORT                                  ####### -->
<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
<xsl:template name="display_test_table">

      <table class="tableContainer">
         <tr align="center">
            <td class="tableHeader" rowspan="2">Test</td>
            
            <xsl:for-each select="/TestReports/test_list/group[@name=$gGroupName]">
               <xsl:call-template name="display_configuration">
                  <xsl:with-param name="targetName" select="../@platform"/>
                  <xsl:with-param name="productPackageName" select="../@tbase_version"/>
                  <xsl:with-param name="testPackageName" select="../@test_package_version"/>
                  <xsl:with-param name="testSuiteName" select="../@test_suite_name"/>
                  <xsl:with-param name="binaries_version" select="../@binaries_version"/>
                  <xsl:with-param name="date" select="../@date"/>
               </xsl:call-template>
            </xsl:for-each>
         </tr>

         <tr align="center" class="tableHeader">
            <xsl:for-each select="/TestReports/test_list/group[@name=$gGroupName]">
               <td>
                  Results
               </td>
               <td>
                  Doc.
               </td>
               
               <!-- if add_exec_time was sent as parameter, show this column -->
               <xsl:if test="/TestReports/display/@execution_time='True'">
                  <td>
                     Time (s)
                  </td>
               </xsl:if>
            </xsl:for-each>
         </tr>
         
         
         <!-- The row that shows the total metrics -->
         <tr align="center">
            <td class="tableDescr">
               Total metrics
            </td>

            <xsl:for-each select="/TestReports/test_list/group[@name=$gGroupName]">
               <td class="tableSummarize">
                  <xsl:call-template name="display_group_total">
                     <xsl:with-param name="totalTC" select="count(./test/test_case)"/>
                     <xsl:with-param name="totalTCSuccess" select="count(./test/test_case[@result='SUCCESS'])"/>
                     <xsl:with-param name="totalTCFailure" select="count(./test/test_case[@result='FAILURE'])"/>
                     <xsl:with-param name="totalTCKnownFailure" select="count(./test/test_case[@result='KNOWN_FAILURE'])"/>
                     <xsl:with-param name="totalTCWarning" select="count(./test/test_case[@result='SHOULD_FAIL_BUT_SUCCESS'])"/>
                     <xsl:with-param name="totalTCNotExecuted" select="count(./test/test_case[@result='NOT_EXECUTED'])"/>
                     <xsl:with-param name="totalTCUndocumented" select="count(./test/test_case[@documented='NOT_DOCUMENTED'])"/>
                     <xsl:with-param name="nbCrash" select="count(./test[@status='CRASH'])"/>
                     <xsl:with-param name="nbTimeout" select="count(./test[@status='TIMEOUT'])"/>
                  </xsl:call-template>
               </td>
               <td class="tableSummarize"/>
               <xsl:if test="/TestReports/display/@execution_time='True'">
                  <td class="tableSummarize" align="center">
                     <xsl:call-template name="display_time_total">
                        <xsl:with-param name="sum_time_total" select="sum(./test/@time_in_ms)"/>
                     </xsl:call-template>
                  </td>
               </xsl:if>
            </xsl:for-each>
         </tr>

         <xsl:for-each select="/TestReports/test_list/group[@name=$gGroupName]/test[not(preceding::group[@name=$gGroupName]/test/@name = @name)]">
            <xsl:variable name="testName" select="@name" />

            <tr>
               <td class="tableDescr">
                  <a>
                     <xsl:attribute name="href">
                        <xsl:value-of select="concat($htmlPageName,'?group=',$gGroupName,'&amp;test=',$testName)" />
                     </xsl:attribute>
                     <xsl:value-of select="$testName"/>
                  </a>
               </td>

               <xsl:for-each select="/TestReports/test_list/group[@name=$gGroupName]">
                        
                  <td class="tableContent" align="center">
                     <xsl:call-template name="display_test_total">
                        <xsl:with-param name="targetName" select="../@target"/>
                        <xsl:with-param name="gGroupName" select="$gGroupName"/>
                        <xsl:with-param name="testName" select="$testName"/>
                        <xsl:with-param name="totalTC" select="count(./test[@name=$testName]/test_case)"/>
                        <xsl:with-param name="totalTCSuccess" select="count(./test[@name=$testName]/test_case[@result='SUCCESS'])"/>
                        <xsl:with-param name="totalTCFailure" select="count(./test[@name=$testName]/test_case[@result='FAILURE'])"/>
                        <xsl:with-param name="totalTCNotExecuted" select="count(./test[@name=$testName]/test_case[@result='NOT_EXECUTED'])"/>
                        <xsl:with-param name="totalTCWarning" select="count(./test[@name=$testName]/test_case[@result='SHOULD_FAIL_BUT_SUCCESS'])"/>
                        <xsl:with-param name="totalTCKnownFailure" select="count(./test[@name=$testName]/test_case[@result='KNOWN_FAILURE'])"/>
                        <xsl:with-param name="totalTCUndocumented" select="count(./test[@name=$testName]/test_case[@documented='NOT_DOCUMENTED'])"/>
                        <xsl:with-param name="status" select="./test[@name=$testName]/@status"/>
                        <xsl:with-param name="testBinaryName" select="../@test_suite_name"/>
                        <xsl:with-param name="isBlacklisted" select="./test[@name=$testName]/@blacklisted"/>
                        <xsl:with-param name="res_path" select="../@res_path"/>
                     </xsl:call-template>
                  </td>
                  
                  <xsl:variable name="totalTCBlacklist" select="count(./test[@name=$testName and @blacklisted]/test_case)"/>
                  <xsl:variable name="totalTCBDoc" select="count(./test[@name=$testName and not(@blacklisted)]/test_case)"/>
                  
                  <td class="tableContent" align="center">
                     <xsl:call-template name="display_doc_link">
                        <xsl:with-param name="blacklistLink" select="concat(../@res_path, '/doc/', ../@test_suite_name , '_BlackList.html/', ./test[@name=$testName]/@doc_ref, '.html')"/>
                        <xsl:with-param name="docLink" select="concat(../@res_path, '/doc/', ../@test_suite_name , '.html/', ./test[@name=$testName]/@doc_ref, '.html')"/>
                        <xsl:with-param name="isdocDocumented">
                           <xsl:if test="./test[@name=$testName]/@doc_ref">
                              Ok
                           </xsl:if>
                        </xsl:with-param>
                        <xsl:with-param name="isBlacklistDocumented">
                           <xsl:if test="./test[@name=$testName]/@doc_ref">
                              Ok
                           </xsl:if>
                        </xsl:with-param>
                        <xsl:with-param name="displayBlacklist">
                           <xsl:if test="$totalTCBlacklist != 0">
                              Ok
                           </xsl:if>
                        </xsl:with-param>
                        <xsl:with-param name="displayDoc">
                           <xsl:if test="$totalTCBDoc != 0">
                              Ok
                           </xsl:if>
                        </xsl:with-param>
                     </xsl:call-template>
                  </td>
            
            
                  <xsl:if test="/TestReports/display/@execution_time='True'">
                     <td class="tableContent" align="center">
                        <xsl:call-template name="display_time_total">
                           <xsl:with-param name="sum_time_total" select="sum(./test[@name=$testName]/@time_in_ms)"/>
                        </xsl:call-template>
                     </td>
                  </xsl:if>            
               </xsl:for-each>
            </tr>
         </xsl:for-each>

         <tr>
            <xsl:call-template name="display_links">
               <xsl:with-param name="isBackToIndex" select="'True'"/>
               <xsl:with-param name="BackToGoup" select="''"/>
            </xsl:call-template>
         </tr>
      </table>
</xsl:template>
<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
<!-- #######                            TEST CASE REPORT                                ####### -->
<!-- ########################################################################################## -->
<!-- ########################################################################################## -->
<xsl:template name="display_test_case_table">
   <table class="tableContainer">
   <tr align="center">
      <td class="tableHeader" rowspan="2">Test</td>
      
	   <xsl:for-each select="/TestReports/test_list/group[@name=$gGroupName]/test[@name=$gTestName]">
         <xsl:call-template name="display_configuration">
            <xsl:with-param name="targetName" select="../../@platform"/>
            <xsl:with-param name="productPackageName" select="../../@tbase_version"/>
            <xsl:with-param name="testPackageName" select="../../@test_package_version"/>
            <xsl:with-param name="testSuiteName" select="../../@test_suite_name"/>
            <xsl:with-param name="binaries_version" select="../../@binaries_version"/>
            <xsl:with-param name="date" select="../../@date"/>
         </xsl:call-template>
      </xsl:for-each>
   </tr>

   <tr align="center" class="tableHeader">
      <xsl:for-each select="/TestReports/test_list/group[@name=$gGroupName]/test[@name=$gTestName]">
         <td>
            Results
         </td>
         <td>
            Doc.
         </td>
            
         <!-- if add_exec_time was sent as parameter, show this column -->
         <xsl:if test="/TestReports/display/@execution_time='True'">
            <td>
               Time (s)
            </td>
         </xsl:if>
      </xsl:for-each>
   </tr>
         
   <!-- The row that shows the total metrics -->
   <tr align="center">
      <td class="tableDescr">
         Total metrics
      </td>

      <xsl:for-each select="/TestReports/test_list/group[@name=$gGroupName]/test[@name=$gTestName]">
         <td class="tableSummarize">
            <xsl:call-template name="display_test_total">
               <xsl:with-param name="targetName" select="../../@target"/>
               <xsl:with-param name="gGroupName" select="$gGroupName"/>
               <xsl:with-param name="testName" select="$gTestName"/>
               <xsl:with-param name="totalTC" select="count(./test_case)"/>
               <xsl:with-param name="totalTCSuccess" select="count(./test_case[@result='SUCCESS'])"/>
               <xsl:with-param name="totalTCFailure" select="count(./test_case[@result='FAILURE'])"/>
               <xsl:with-param name="totalTCNotExecuted" select="count(./test_case[@result='NOT_EXECUTED'])"/>
               <xsl:with-param name="totalTCWarning" select="count(./test_case[@result='SHOULD_FAIL_BUT_SUCCESS'])"/>
               <xsl:with-param name="totalTCKnownFailure" select="count(./test_case[@result='KNOWN_FAILURE'])"/>
               <xsl:with-param name="totalTCUndocumented" select="count(./test_case[@documented='NOT_DOCUMENTED'])"/>
               <xsl:with-param name="status" select="./@status"/>
               <xsl:with-param name="testBinaryName" select="../../@test_suite_name"/>
               <xsl:with-param name="isBlacklisted" select="./@blacklisted"/>
               <xsl:with-param name="res_path" select="../../@res_path"/>
            </xsl:call-template>
         </td>
         <td/>
         <xsl:if test="/TestReports/display/@execution_time='True'">
            <td class="tableSummarize">
               <xsl:call-template name="display_time_total">
                  <xsl:with-param name="sum_time_total" select="sum(./@time_in_ms)"/>
               </xsl:call-template>
            </td>
         </xsl:if>
      </xsl:for-each>
   </tr>

   <xsl:for-each select="/TestReports/test_list/group[@name=$gGroupName]/test[@name=$gTestName]/test_case[not(preceding::group[@name=$gGroupName]/test[@name=$gTestName]/test_case/@name = @name)]">
      <xsl:variable name="testcaseName" select="@name" />
      <tr>
         <td class="tableDescr">
            <b><xsl:value-of select="$testcaseName"/></b>
         </td>
         
         <xsl:for-each select="/TestReports/test_list/group[@name=$gGroupName]/test[@name=$gTestName]">
            <td class="tableContent" align="center">
               <xsl:call-template name="display_test_case_total">
                  <xsl:with-param name="testcaseResult" select="./test_case[@name=$testcaseName]/@result"/>
                  <xsl:with-param name="testcaseDoc" select="./test_case[@name=$testcaseName]/@documented"/>
                  <xsl:with-param name="testcaseMsg" select="./test_case[@name=$testcaseName]/message/."/>
                  <xsl:with-param name="testcaseBugMsg" select="./test_case[@name=$testcaseName]/description/."/>
                  <xsl:with-param name="testcaseBugLink" select="./test_case[@name=$testcaseName]/@issue_ID"/>
               </xsl:call-template>
            </td>
            
            <td class="tableContent" align="center">
               <xsl:call-template name="display_doc_link">
                  <xsl:with-param name="blacklistLink" select="concat(../../@res_path, '/doc/', ../../@test_suite_name , '_BlackList.html/', @doc_ref, '.html#', ./test_case[@name=$testcaseName]/@doc_ref)"/>
                  <xsl:with-param name="docLink" select="concat(../../@res_path, '/doc/', ../../@test_suite_name , '.html/', @doc_ref, '.html#', ./test_case[@name=$testcaseName]/@doc_ref)"/>
                  <xsl:with-param name="isdocDocumented">
                     <xsl:if test="./test_case[@name=$testcaseName]/@documented = 'DOCUMENTED'">
                        Ok
                     </xsl:if>
                  </xsl:with-param>
                  <xsl:with-param name="isBlacklistDocumented">
                     <xsl:if test="./test_case[@name=$testcaseName]/@documented = 'DOCUMENTED'">
                        Ok
                     </xsl:if>
                  </xsl:with-param>
                  <xsl:with-param name="displayBlacklist">
                     <xsl:if test="./@blacklisted = 'true'">
                        Ok
                     </xsl:if>
                  </xsl:with-param>
                  <xsl:with-param name="displayDoc">
                     <xsl:if test="not(./@blacklisted)">
                        Ok
                     </xsl:if>
                  </xsl:with-param>
               </xsl:call-template>
            </td>
            
            <xsl:if test="/TestReports/display/@execution_time='True'">
               <td class="tableContent" align="center">
                  <xsl:call-template name="display_time_total">
                     <xsl:with-param name="sum_time_total" select="./test_case[@name=$testcaseName]/@time_in_ms"/>
                  </xsl:call-template>
               </td>
            </xsl:if>
            
            
         </xsl:for-each>
      </tr>
   </xsl:for-each> 

   <tr>
      <xsl:call-template name="display_links">
         <xsl:with-param name="isBackToIndex" select="'True'"/>
         <xsl:with-param name="BackToGoup" select="$gGroupName"/>
      </xsl:call-template>
   </tr>
   </table>
</xsl:template>
</xsl:stylesheet>
