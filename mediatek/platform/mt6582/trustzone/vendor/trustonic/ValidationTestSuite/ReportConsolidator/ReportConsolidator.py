#
# Copyright (c) 2013 TRUSTONIC LIMITED
# All rights reserved
# 
#  The present software is the confidential and proprietary information of
#  TRUSTONIC LIMITED. You shall not disclose the present software and shall
#  use it only in accordance with the terms of the license agreement you
#  entered into with TRUSTONIC LIMITED. This software may be subject to
#  export or import laws in certain countries.
# 

#--------------------------------------------------------------
#              IMPORTS
#-------------------------------------------------------------
import sys                    #for getting param/ report crash...
import os                     #for file manipulation
import shutil                 #for file manipulation
import argparse               #for parsing program parameters
import traceback              #for dump info in case of crash
import re                     #for regular expression support
from xml.dom import minidom   #for xml parsing
from lxml import etree        #for .html static generation


#---------------------------------------------------------------------------
#      Function that create the display node of the test report
#---------------------------------------------------------------------------
def createDisplayNode(xmlFile, isCustomerVers):

   # Create the <display> node
   displayNode = xmlFile.createElement("display")
   displayNode.setAttribute("benchmark", "False")
   if(not isCustomerVers):
      displayNode.setAttribute("execution_time", "True")
      displayNode.setAttribute("jira_link", "True")
   xmlFile.documentElement.appendChild(displayNode)
   

#---------------------------------------------------------------------------
#      Function that create the report xml file
#---------------------------------------------------------------------------
def generateOutputXML(outputXMLList, doc, isCustomerVers):
   # Create the output document
   xmloutput = minidom.Document()
   
   # Create the <TestReports> base element
   TestReportsnode = xmloutput.appendChild(xmloutput.createElement("TestReports"))
   
   for xmlFiles in outputXMLList:
      for testlistNode in xmlFiles.getElementsByTagName("test_list"):
         testlistNode.setAttribute("res_path", doc)
         xmloutput.documentElement.appendChild(testlistNode)
      
   createDisplayNode(xmloutput, isCustomerVers)
   return xmloutput
   

#---------------------------------------------------------------------------
#      Function that displays test cases global statistics
#---------------------------------------------------------------------------
def displayStatistics(foldername, xmlFile):
   gnbTC = 0
   gnbTCSuccess = 0
   gnbTCFailure = 0
   gnbTcKnownFailure = 0
   
   for testcase in xmlFile.getElementsByTagName("test_case"):
      gnbTC += 1
      testcaseresult = testcase.getAttribute("result")
      
      if(testcaseresult=="SUCCESS"):
         gnbTCSuccess += 1
      elif(testcaseresult=="FAILURE"):
         gnbTCFailure += 1
      elif(testcaseresult=="KNOWN_FAILURE"):
         gnbTcKnownFailure += 1
         gnbTCFailure += 1
      elif(testcaseresult=="SHOULD_FAIL_BUT_SUCCESS"):
         gnbTCSuccess += 1
      #else: NOT_EXECUTED or state unknown
               
   #display global results:
   
   print '+--------------------------------------------------------------------------'
   print '| Results generated in '+foldername
   print '+--------------------------------------------------------------------------'
   print '| Nb test cases          : {:>4}'.format(gnbTC)
   print '| Nb Executed test cases : {:>4}'.format(gnbTCSuccess + gnbTCFailure)
   print '| Successful test cases  : {:>4}'.format(gnbTCSuccess)
   print '| Failed test cases      : {:>4}'.format(gnbTCFailure)
   print '| Known failures         : {:>4}'.format(gnbTcKnownFailure)
   print '+--------------------------------------------------------------------------'
   
   #return an error code in the case of an unexpected FAILURE
   if gnbTCFailure != gnbTcKnownFailure:
      return False

   return True

#---------------------------------------------------------------------------
#       Helper functions to create/get a xml node
#---------------------------------------------------------------------------
def getTestListNode(xml, product_package_val, target_val, test_package_val, test_suite_name_val, binaries_version, date, createifmissing):

   for testlist in xml.getElementsByTagName("test_list"):
      if (testlist.getAttribute("tbase_version") == product_package_val) \
         and (testlist.getAttribute("platform") == target_val) \
         and (testlist.getAttribute("test_suite_name") == test_suite_name_val) \
         and (testlist.getAttribute("binaries_version") == binaries_version) \
         and (testlist.getAttribute("date") == date) \
         and (testlist.getAttribute("test_package_version") == test_package_val):
            return testlist

   if(createifmissing):
      testListNode = xml.createElement("test_list")
      testListNode.setAttribute("tbase_version", product_package_val)
      testListNode.setAttribute("platform", target_val)
      testListNode.setAttribute("test_suite_name", test_suite_name_val)
      testListNode.setAttribute("binaries_version", binaries_version)
      testListNode.setAttribute("date", date)
      testListNode.setAttribute("test_package_version", test_package_val)
      
      return xml.documentElement.appendChild(testListNode) 
   return None

def getGroupNode(xml, testlistNode, groupname, createifmissing):

   for group in testlistNode.getElementsByTagName("group"):
      if (group.getAttribute("name") == groupname):
         return group
   
   if(createifmissing):
      groupNode = xml.createElement("group")
      groupNode.setAttribute("name", groupname)
      return testlistNode.appendChild(groupNode)
         
   return None
      
      
def getTestNode(xml, testlistNode, groupname, testname, createifmissing):

   group = getGroupNode(xml, testlistNode, groupname, createifmissing)
   if (group == None):
      return None
   
   for test in group.getElementsByTagName("test"):
      if (test.getAttribute("name") == testname):
         return test

   if(createifmissing):
      testNode = xml.createElement("test")
      testNode.setAttribute("name", testname)
      return group.appendChild(testNode)
      
   return None
   
   
def getTestCaseNode(xml, testlistNode, groupname, testname, testcasename, createifmissing):

   test = getTestNode(xml, testlistNode, groupname, testname, createifmissing)
   if (test == None):
      return None
   
   for testcase in test.getElementsByTagName("test_case"):
      if (testcase.getAttribute("name") == testcasename):
         return testcase

   if(createifmissing):
      testcaseNode = xml.createElement("test_case")
      testcaseNode.setAttribute("name", testcasename)
      return test.appendChild(testcaseNode)
         
   return None
   

#---------------------------------------------------------------------------
#        Function that merge all data from different report
#---------------------------------------------------------------------------
def testReportFusion(testsreportList):
   #Create the output document
   consolidatedXML = minidom.Document()
   mainNode = consolidatedXML.appendChild(consolidatedXML.createElement("TestReports"))
   
   for testsreport in testsreportList:
      testsreportPath = testsreport[0] #os.path.basename(os.path.normpath(testsreport[0])) FSY
      testsreportRelDir = os.path.basename(os.path.normpath(os.path.split(testsreportPath)[0]))
      testsreportXML  = testsreport[1]
      for testlist in testsreportXML.getElementsByTagName("test_list"):
         testlist.setAttribute("res_path", testsreportRelDir)
         mainNode.appendChild(testlist.cloneNode(True))
      
   return consolidatedXML
   
#---------------------------------------------------------------------------
#        Function that get usefull informations from result file
#---------------------------------------------------------------------------
def testResultsFusion(testsresultsXMLList):
   #Create the output document
   consolidatedXML = minidom.Document()
   mainNode = consolidatedXML.appendChild(consolidatedXML.createElement("TestReports"))
   
   for testsresultsXML in testsresultsXMLList:
      #Find the test list node if already exists, else create it
      product_package_val = testsresultsXML.documentElement.getAttribute("tbase_version")
      target_val = testsresultsXML.documentElement.getAttribute("platform")
      test_package_val = testsresultsXML.documentElement.getAttribute("test_package_version")
      binaries_version = testsresultsXML.documentElement.getAttribute("binaries_version")
      date = testsresultsXML.documentElement.getAttribute("date")
      test_suite_name_val = testsresultsXML.documentElement.getAttribute("test_suite_name")
      isBlackListed =  test_suite_name_val.endswith("_BlackList")
      test_suite_name_val = test_suite_name_val.replace("_BlackList", "")
      
      testlistNode = getTestListNode(consolidatedXML, product_package_val, target_val, test_package_val, test_suite_name_val, binaries_version, date, True)
      
      #Copy each test case
      for test in testsresultsXML.getElementsByTagName("test"):
         testname = test.getAttribute("name")
         groupname = test.parentNode.getAttribute("name")
            
         #Create the test node
         testnode = getTestNode(consolidatedXML, testlistNode, groupname, testname, createifmissing=True)
         testnode.setAttribute("time_in_ms", test.getAttribute("time_in_ms"))
         
         test_status = test.getAttribute("status")
         if test_status!="":
            #Set test node attribute
            testnode.setAttribute("status", test_status)
      
         if isBlackListed:
            testnode.setAttribute("blacklisted", "true")
               
         for testcase in test.getElementsByTagName("test_case"):
            testcasename = testcase.getAttribute("name")
            
            #Create the test case node
            testcasenode = getTestCaseNode(consolidatedXML, testlistNode, groupname, testname, testcasename, createifmissing=True)
            #Set test case node attributes
            testcasenode.setAttribute("result", testcase.getAttribute("result"))
            testcasenode.setAttribute("time_in_ms", testcase.getAttribute("time_in_ms"))
            for elmt in testcase.getElementsByTagName("message"):
               testcasenode.appendChild(elmt.cloneNode(True))
      
   return consolidatedXML
           
#---------------------------------------------------------------------------
#        Function that get usefull informations from documentation file
#---------------------------------------------------------------------------
def testsDocumentedFusion(testsDocumentedXMLList):
   #Create the output document
   consolidatedXML = minidom.Document()
   mainNode = consolidatedXML.appendChild(consolidatedXML.createElement("TestDocumented"))
   
   for testsDocumentedXML in testsDocumentedXMLList:
      platform_val = testsDocumentedXML.documentElement.getAttribute("platform")
      test_suite_name_val = testsDocumentedXML.documentElement.getAttribute("test_suite_name")
      isBlackListed =  test_suite_name_val.endswith("_BlackList")
      test_suite_name_val = test_suite_name_val.replace("_BlackList", "")
      testsDocumentedXML.documentElement.setAttribute("test_suite_name", test_suite_name_val)
      
      #Check if a node already exist or not
      documentationNode=None
      for producttestlist in consolidatedXML.getElementsByTagName("product_test_list"):
         if (producttestlist.getAttribute("test_suite_name") == test_suite_name_val) \
              and (producttestlist.getAttribute("platform").lower() == platform_val.lower()) :
            documentationNode = producttestlist
            break
      
      if documentationNode==None: #does not exist, so create it
         documentationNode = consolidatedXML.createElement("product_test_list")
         documentationNode.setAttribute("platform", platform_val)
         documentationNode.setAttribute("test_suite_name", test_suite_name_val)
         consolidatedXML.documentElement.appendChild(documentationNode) 
         
      for group in testsDocumentedXML.getElementsByTagName("group"):
         groupname = group.getAttribute("name")
   
         #Create the group node
         groupnode = getGroupNode(consolidatedXML, documentationNode, groupname, createifmissing=True)
         
         #Set test node attributes
         groupnode.setAttribute("doc_ref", group.getAttribute("doc_ref"))
            
         #Copy each test case
         for test in group.getElementsByTagName("test"):
            testname = test.getAttribute("name")
               
            #Create the test node
            testnode = getTestNode(consolidatedXML, documentationNode, groupname, testname, createifmissing=True)
            
            #Set test node attributes
            testnode.setAttribute("doc_ref", test.getAttribute("doc_ref"))
            if isBlackListed:
               testnode.setAttribute("blacklisted", "true")

            for testcase in test.getElementsByTagName("test_case"):
               testcasename = testcase.getAttribute("name")
               
               #Create the test case node
               testcasenode = getTestCaseNode(consolidatedXML, documentationNode, groupname, testname, testcasename, createifmissing=True)
               
               #Set test case node attributes
               testcasenode.setAttribute("doc_ref", testcase.getAttribute("doc_ref"))
      
   return consolidatedXML
   
#---------------------------------------------------------------------------
#     Function that consolidate tests results with documentation
#---------------------------------------------------------------------------   
def checkResultsAgainstPTL(consolidatedXML, documentationXML):
   
   for testListNode in consolidatedXML.getElementsByTagName("test_list"):
      ##################################
      #Get the associated documentation#
      ##################################
      documentationNode=None
      test_suite_name_val = testListNode.getAttribute("test_suite_name")
      platform_val = testListNode.getAttribute("platform").split(",")[0].strip()
      for producttestlist in documentationXML.getElementsByTagName("product_test_list"):
         if (producttestlist.getAttribute("test_suite_name") == test_suite_name_val) \
              and (producttestlist.getAttribute("platform").lower() == platform_val.lower()
                   or producttestlist.getAttribute("platform").lower()=="all") :
            documentationNode = producttestlist
            break
      
      if documentationNode==None:
         print "WARNING: documentation not found for platform: '"+platform_val+"' and test suite '"+test_suite_name_val+"'"
         #Create a dummy XML to allow the rest of processing...each test will be tagged as undocumented
         xmloutput = minidom.Document()
         documentationNode = xmloutput.createElement("product_test_list")
      
      ##########################################
      #Check testsresults against documentation#
      ##########################################
      #Check that each group present in tests results is documented
      for test in testListNode.getElementsByTagName("group"):
         groupname = test.getAttribute("name")
         
         #Get the test doc_ref if exists
         docgroupnode = getGroupNode(documentationXML, documentationNode, groupname, createifmissing=False)
         if (docgroupnode != None):
            test.setAttribute("doc_ref", docgroupnode.getAttribute("doc_ref")) 
            
      #Check that each test present in tests results is documented
      for test in testListNode.getElementsByTagName("test"):
         testname = test.getAttribute("name")

         groupname = test.parentNode.getAttribute("name")
         
         #Get the test doc_ref if exists
         doctestnode = getTestNode(documentationXML, documentationNode, groupname, testname, createifmissing=False)
         if (doctestnode != None):
            test.setAttribute("doc_ref", doctestnode.getAttribute("doc_ref"))
            
      #Get documentation informations of each test case
      for testcase in testListNode.getElementsByTagName("test_case"):
         testcasename  = testcase.getAttribute("name")
         testname      = testcase.parentNode.getAttribute("name")
         groupname     = testcase.parentNode.parentNode.getAttribute("name")
         isBlacklisted = (testcase.parentNode.getAttribute("blacklisted")=="true")
         
         #Get documentation
         doctestcasenode = getTestCaseNode(documentationXML, documentationNode, groupname, testname, testcasename, createifmissing=False)
         if (doctestcasenode != None):
            doctestcaseBL=(doctestcasenode.parentNode.getAttribute("blacklisted")=="true")
            if(doctestcaseBL==isBlacklisted):
               #In documentation
               testcase.setAttribute("documented", "DOCUMENTED")
               #copy doc_ref attribute
               testcase.setAttribute("doc_ref", doctestcasenode.getAttribute("doc_ref"))
            else:
               #Not in documentation
               testcase.setAttribute("documented", "NOT_DOCUMENTED")
         else:
            #Not in documentation
            testcase.setAttribute("documented", "NOT_DOCUMENTED")
               
      ##########################################
      #Check documentation against testsresults#
      ##########################################
      #Check that each test case present in documentation has been executed
      for group in documentationNode.getElementsByTagName("group"):
         groupname = group.getAttribute("name")
   
         #Create the group node
         groupnode = getGroupNode(consolidatedXML, testListNode, groupname, createifmissing=True)
         #Set test node attributes
         groupnode.setAttribute("doc_ref", group.getAttribute("doc_ref"))
            
         #Copy each test case
         for test in group.getElementsByTagName("test"):
            testname = test.getAttribute("name")
               
            #Create the test node
            testnode = getTestNode(consolidatedXML, testListNode, groupname, testname, createifmissing=True)
            #Set test node attributes
            testnode.setAttribute("doc_ref", test.getAttribute("doc_ref"))
            if (test.getAttribute("blacklisted")=="true"):
               testnode.setAttribute("blacklisted", "true")

            for testcase in test.getElementsByTagName("test_case"):
               testcasename  = testcase.getAttribute("name")
               
               constestcasenode = getTestCaseNode(consolidatedXML, testListNode, groupname, testname, testcasename, createifmissing=False)
               if(constestcasenode == None):
                  #Need to be called twice: first time with createifmissing=False and second one with createifmissing=True
                  #in order to know if the node was created or not
                  constestcasenode = getTestCaseNode(consolidatedXML, testListNode, groupname, testname, testcasename, createifmissing=True)
                  #Set all needed informations for the test case
                  constestcasenode.setAttribute("name", testcase.getAttribute("name"))
                  constestcasenode.setAttribute("doc_ref", testcase.getAttribute("doc_ref"))
                  constestcasenode.setAttribute("result", "NOT_EXECUTED")
                  constestcasenode.setAttribute("documented", "DOCUMENTED")

   #####################################################
   #Check that all documented tests have been processed#
   #####################################################
   for documentationNode in documentationXML.getElementsByTagName("product_test_list"):
      ##################################
      #Get the associated testsresults #
      ##################################
      testreportNode=None
      test_suite_name_val = documentationNode.getAttribute("test_suite_name")
      platform_val = documentationNode.getAttribute("platform")
      
      for testListNode in consolidatedXML.getElementsByTagName("test_list"):
         if (testListNode.getAttribute("test_suite_name") == test_suite_name_val) \
              and (testListNode.getAttribute("platform").split(",")[0].strip().lower() == platform_val.lower()
                   or platform_val.lower()=="all") :
            testreportNode = testListNode
            break
      
      #if not found create empty results
      if testreportNode==None:
         #Create a new node for unexecuted tests
         testreportNode = consolidatedXML.createElement("test_list")
         testreportNode.setAttribute("tbase_version", "Not executed")
         testreportNode.setAttribute("platform", platform_val)
         testreportNode.setAttribute("test_suite_name", test_suite_name_val)
         testreportNode.setAttribute("binaries_version", "Not executed")
         testreportNode.setAttribute("date", "Not executed")
         testreportNode.setAttribute("test_package_version", "Not executed")
         consolidatedXML.documentElement.appendChild(testreportNode) 
      
         for group in documentationNode.getElementsByTagName("group"):
            groupname = group.getAttribute("name")
      
            #Create the group node
            groupnode = getGroupNode(consolidatedXML, testreportNode, groupname, createifmissing=True)
            #Set test node attributes
            groupnode.setAttribute("doc_ref", group.getAttribute("doc_ref"))
               
            #Copy each test case
            for test in group.getElementsByTagName("test"):
               testname = test.getAttribute("name")
                  
               #Create the test node
               testnode = getTestNode(consolidatedXML, testreportNode, groupname, testname, createifmissing=True)
               #Set test node attributes
               testnode.setAttribute("doc_ref", test.getAttribute("doc_ref"))
               if (test.getAttribute("blacklisted")=="true"):
                  testnode.setAttribute("blacklisted", "true")

               for testcase in test.getElementsByTagName("test_case"):
                  testcasename  = testcase.getAttribute("name")
                  
                  constestcasenode = getTestCaseNode(consolidatedXML, testreportNode, groupname, testname, testcasename, createifmissing=True)
                  #Set all needed informations for the test case
                  constestcasenode.setAttribute("name", testcase.getAttribute("name"))
                  constestcasenode.setAttribute("doc_ref", testcase.getAttribute("doc_ref"))
                  constestcasenode.setAttribute("result", "NOT_EXECUTED")
                  constestcasenode.setAttribute("documented", "DOCUMENTED")
            
            
#---------------------------------------------------------------------------
#                 Function to get conformance file informations
#---------------------------------------------------------------------------
def applyConformance(testsresultsXML, conformanceXML):

   #Analyse all tests cases in the conformance file
   for expectedFailure in conformanceXML.getElementsByTagName("expected_failure"):
      groupname = expectedFailure.getAttribute("group_name")
      testname = expectedFailure.getAttribute("test_name")
      testcasename = expectedFailure.getAttribute("test_case_name")

      #Check if the test case exists in the tests results
      for testListNode in testsresultsXML.getElementsByTagName("test_list"):
         resulttestcasenode = getTestCaseNode(testsresultsXML, testListNode, groupname, testname, testcasename, createifmissing=False)
         #if(resulttestcasenode == None):
         #   #No: This case should not exists
         #   print "/!\\ WARNING /!\\"
         #   print " Test case %s.%s.%s in conformance file " \
         #         "but not in documentation nor tests results" %(groupname, testname, testcasename)
         #else:
         if(resulttestcasenode != None):
            #Yes: update the tests results accordingly to the conformance file
            resultstate = resulttestcasenode.getAttribute("result")
            if(resultstate == "SUCCESS"):
               resulttestcasenode.setAttribute("result", "SHOULD_FAIL_BUT_SUCCESS")
            elif(resultstate == "FAILURE"):
               resulttestcasenode.setAttribute("result", "KNOWN_FAILURE")
            
            resulttestcasenode.setAttribute("issue_ID", expectedFailure.getAttribute("issue_ID"))
            resulttestcasenode.setAttribute("severity", expectedFailure.getAttribute("severity"))
            
            for elmt in expectedFailure.getElementsByTagName("description"):
               resulttestcasenode.appendChild(elmt.cloneNode(True))
               

#---------------------------------------------------------------------------
#                 Function to parse "properly" a XML file
#---------------------------------------------------------------------------
def openXmlFile(fileName, fileDescr):
   try :
      return minidom.parse(fileName)
   except Exception, err:
    print "ERROR: Could not parse ", fileDescr," file"
    print traceback.format_exc()
    sys.exit(1)
    
#---------------------------------------------------------------------------
#        Function to get all XML file from a list of file or directory
#---------------------------------------------------------------------------
def openXmlMultiFiles(dirList, xmlPattern, fileDescr):
   
   xmlList=[]
   for item in dirList:
      if(os.path.isfile(item)):
         #A specific test results file is defined
         xmlData = openXmlFile(item, fileDescr +" "+item)
         xmlList.append(xmlData)
      else:
         #A path for one or more test results files is defined
         for root, dirs, files in os.walk(item): 
            for filename in files: 
               #print "check "+filename+" with "+xmlPattern
               if(re.search(xmlPattern, filename)):
                  xmlData = openXmlFile(os.path.join(root, filename), fileDescr +" "+filename)
                  xmlList.append(xmlData)
   return xmlList
   
#---------------------------------------------------------------------------
# Function to copy all files needed for displaying the report consolidator
#---------------------------------------------------------------------------
def copyRessourceFiles(outDir):
   #Get the path of this script file
   relativeScriptPath = os.path.dirname(os.path.abspath(sys.argv[0]))
   
   #If the output directory is not in the same directory: copy all the needed files
   if(not os.path.samefile(relativeScriptPath, outDir)):
      #Delete ressource directory if allready exists
      if os.path.exists(os.path.join(outDir,"conf")):
         shutil.rmtree(os.path.join(outDir,"conf"))
      #Copy all ressource files   
      shutil.copytree(os.path.join(relativeScriptPath,"conf"), os.path.join(outDir,"conf"))
      shutil.copy(os.path.join(relativeScriptPath,"index.html"), outDir)
      
#---------------------------------------------------------------------------
#                 Program parameters parsing function
#---------------------------------------------------------------------------
def check_parameters(argv):

   default_result_dir = "./"+os.path.relpath(os.path.join(os.path.dirname(argv[0]), "..", "..", "results"), os.getcwd())
   #Create the parameters parser
   parser = argparse.ArgumentParser(description='Consolidate the test results in a human readable format')
   parser.add_argument('-r', '--results', default=default_result_dir, help="results path(default: '%(default)s')")
   
   #Parse input parameters
   args = parser.parse_args()
   
   #Check file parameters
   if not os.path.isdir(args.results):
      parser.error("%s directory does not exists" % args.results)
   
   return args     

def generateCSV(xml, outdir):
   for testlistNode in xml.getElementsByTagName("test_list"):
      tbase_version = testlistNode.getAttribute("tbase_version")
      test_suite_name = testlistNode.getAttribute("test_suite_name")
      
      #use a list to be able to sort results by alphabetical order
      result_list = []
      
      for testcase in testlistNode.getElementsByTagName("test_case"):
         testcasename = testcase.getAttribute("name")
         testname     = testcase.parentNode.getAttribute("name")
         result       = testcase.getAttribute("result")
         blacklist    = testcase.parentNode.getAttribute("blacklisted")
         
         result_string=""
         if(blacklist!="true"):
            if(result=="SUCCESS")or(result=="SHOULD_FAIL_BUT_SUCCESS"):
               result_string="passed"
            elif(result=="FAILURE")or(result=="KNOWN_FAILURE"):
               result_string="FAILED"
            
         #else, unknow results tests...
         
         if(result_string!=""):
            result_list.append(testname+", "+testcasename+", "+result_string)
      
      result_list.sort()
      
      if not os.path.exists(os.path.join(outdir,"misc")):
         os.mkdir(os.path.join(outdir,"misc"))
      
      with open(os.path.join(outdir, "misc", test_suite_name+".csv"), "w") as f:
         for result in result_list:
            f.write(result+"\n")

# Check if the directory passed as parameter contains a test report
def isReportDir(path):
   #check if report.xml file exists
   return os.path.isfile(os.path.join(path, "report.xml"))
  
# Check if the directory passed as parameter contains tests results
def isResultsDir(path):
   doc_found=False
   report_found=False
   
   #check that 'results_' and doc folders exist
   for item in os.listdir(path):
      if os.path.isdir(os.path.join(path, item)):
         if item=="doc":
            doc_found=True
         if item.startswith("reports_"):
            report_found=True

   return (doc_found and report_found)
            
             
#---------------------------------------------------------------------------
#                 MAIN class
#---------------------------------------------------------------------------
def main(argv):
   #Parse and check program options
   args = check_parameters(argv)
      
   results_folders=[] 
   reports_files=[]
   
   if isResultsDir(args.results):
      results_folders.append(args.results) 
   else:
      for filename in os.listdir(args.results):
         path=os.path.join(args.results, filename)
         if os.path.isdir(path):
            if isReportDir(path):
               reports_files.append(os.path.join(path, "report.xml"))
            elif isResultsDir(path):
               results_folders.append(path) 
   
   reportList=[]
   for filename in reports_files:
      xmlData = openXmlFile(filename, "Report "+filename)
      reportList.append([filename, xmlData])
      
   testsStatus=True
   #for each directory containing 'reports_*' and 'doc'
   for results_folder in results_folders:
      print "Generating the report: "+os.path.join(results_folder, "report.xml")+"..."
      
      #Parse report parameter and get content of all files
      testsResultsList = openXmlMultiFiles([results_folder], "_results.xml$" , "tests results")
            
      #Parse testplan parameter and get content of all files
      testsDocumentedList = openXmlMultiFiles([results_folder], "^PTL_.*.xml$", "tests specification")
      
      #Merge all documentation information in a single document
      testsDocumented = testsDocumentedFusion(testsDocumentedList)
      
      #Merge all report information in a single report
      testsConsolidated = testResultsFusion(testsResultsList)
      
      #Consolidate test result with documentation
      checkResultsAgainstPTL(testsConsolidated, testsDocumented)
      
      #Get known failures from conformance file
      conformance_path=os.path.join(results_folder,"doc","conformance.xml")
      if os.path.isfile(conformance_path):
         testsConformance = openXmlFile(conformance_path, "conformance")
         applyConformance(testsConsolidated, testsConformance)
      else:
         print "Warning: Conformance file '"+conformance_path+"' not found."

      #Generate the output xml file
      outputxml = generateOutputXML([testsConsolidated], ".", False)
      with open(os.path.join(results_folder, "report.xml"), "w") as f:
         f.write('\n'.join([line for line in outputxml.toprettyxml(indent="  ", encoding="UTF-8").split('\n') if line.strip()]))
         
      generateCSV(outputxml, results_folder)
      
      #Copy all necessary ressource files to out dir
      copyRessourceFiles(results_folder)
      
      #User friendly display of test cases statistics
      if not displayStatistics(os.path.join(results_folder, "report.xml"), outputxml):
         testsStatus = False
      
      reportList.append([os.path.join(results_folder, "report.xml"), outputxml])
   
   if len(reportList)>1:
      #Merge all consolidated report in a single report
      print "Merging the reports: "+(", ".join([row[0] for row in reportList]))+"..."
      
      mergedxml = testReportFusion(reportList)
      
      createDisplayNode(mergedxml, False)
      with open(os.path.join(args.results, "report.xml"), "w") as f:
         f.write('\n'.join([line for line in mergedxml.toprettyxml(indent="  ", encoding="UTF-8").split('\n') if line.strip()]))
         
      #Copy all necessary ressource files to out dir
      copyRessourceFiles(args.results)
         
      dom = etree.parse(os.path.join(args.results, "report.xml"))
      xslt = etree.parse(os.path.join(args.results, "conf", "report.xsl"))
      transform = etree.XSLT(xslt)
      newdom = transform(dom, gPage="'compare'")

      with open(os.path.join(args.results, "compare.html"), "w") as f:
         f.write("<html>")
         f.write("<body>")
         f.write('<link type="text/css" href="conf/style.css" rel="stylesheet"/>')
         f.write(etree.tostring(newdom, pretty_print=True))
         f.write("</body>")
         f.write("</html>")

      if not displayStatistics(os.path.join(args.results, "report.xml"), mergedxml):
         testsStatus = False

   #TestsStatus is not null if ther is unexpected failures
   return testsStatus
   
#---------------------------------------------------------------------------
#             Calling the MAIN Function                        
#---------------------------------------------------------------------------
if __name__ == "__main__":
   main(sys.argv[0:])
