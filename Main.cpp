/*
Developed by: Michele Busby, Computational Biologist
Broad Technology Labs/CIL
Broad Institute 
Last Update: 2/9/2016

	Adds the UMI to the RX tag of a bam file.
	Starts with the bam file and either:
	
	FASTQ + position of UMI
	FASTQ with UMIs as the last unit after the last semicolon
	
	Works by creating an enormous hashmap in memory so could be a bit crashy.
	
	
	
*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <dirent.h>
#include <errno.h>

#include "api/BamReader.h"
#include "api/BamWriter.h"
#include "Handy.h"


struct doubleString
{
	string s1;
	string s2;

	doubleString(void)
		: s1()
		, s2()
	
	{}
	
};

unsigned int checkErrors();
void displayHelp();
void somethingsGoneWrong(unsigned int);
void getUMIs();
void processBam(string);
void getBamNames();

//Variables taken from the inputs
string outDirectory="";
string FASTQFileName="";
string bamDirectory="";
int startPos=0; //zero based, input is one based and then 1 is subtracted
int umiLength=8; //number of bases in the UMI


map<string, doubleString> umis; //Read name, UMI
vector<string> bamFileNames; //Can run on 1 FASTQ but multiple bams at the same time


using namespace std;
using namespace BamTools;



int main(int argc, char* argv[]) 
{	
	
	//Intialize library of useful things
	Handy h(0);
	
	int optind=1;


	while ((optind < argc) && (argv[optind][0]=='-')) 
	{	
        string sw = argv[optind];
				
		if (sw=="-h") 
		{	
            optind++;
			displayHelp();
			return 1;
        }
		
		else if(optind >=  argc-1 )
		{
			cerr<<"Your final parameter, "<<sw<<" is missing a value."<<endl;
			return 1;
		}

		else if (sw=="-bam")
		{	
            optind++;
			bamFileNames.push_back(argv[optind]);		
			optind++;
        }
			
		
		else if (sw=="-bam_dir")
		{	
            optind++;
			bamDirectory = argv[optind];		
			optind++;
        }
		
		else if (sw=="-out_dir")
		{	
            optind++;
			outDirectory = argv[optind];		
			optind++;
        }
		
		else if (sw=="-fastq")
		{	
            optind++;
			FASTQFileName = argv[optind];	
			optind++;
        }
		else if (sw=="-start_pos")
		{	
            optind++;
			startPos = h.getIntFromString(argv[optind])-1;//One based start position to 0 based sequence
			optind++;			

        }
		else if (sw=="-umi_length")
		{	
            optind++;
			umiLength=h.getIntFromString(argv[optind]);	
			optind++;
			
        }
		else
		{
			cerr<<"Main: Unknown parameter:"<<sw<<endl;
			displayHelp();
			return 1;
		}
	}	
	
	unsigned int errCheck = checkErrors();
	
	if(bamDirectory.length()>0 && bamFileNames.size()==0)
	{
		getBamNames();		
	}

	getUMIs();	
	
	for(int i=0; i<bamFileNames.size(); ++i)
	{
		processBam(bamFileNames[i]);	
	}
	
	cout<<"Done"<<endl;
		
}

void getUMIs()
{
	
	string line;
	string seqName="";
	doubleString UMIpair;
	std::size_t found;
	ifstream fastqFile (FASTQFileName.c_str() );
	int ctr=0;
	
	while ( getline (fastqFile,line) )
	{			
		if(ctr%40000000 == 0)
		{
			cout<<ctr<<" lines of FASTQ file read."<<endl;		
		}
			
		if(ctr%4 == 0)
		{				
			found=line.find(" ");
			if (found!=std::string::npos)
			{
				line=line.substr(0, found);
			}
			
			seqName=line.substr(1);		
		}
	
		if(ctr%4 == 1)
		{					
			UMIpair.s1=line.substr(startPos, umiLength);	
		}
		
		if(ctr%4 == 3)
		{					
			UMIpair.s2=line.substr(startPos, umiLength);
			
			umis.insert(map< string, doubleString >::value_type(seqName, UMIpair));				
		}
		
		++ctr;
	}
	
	
	fastqFile.close();	
	

}

void getBamNames()
{
	DIR *pdir;
	struct dirent *pent;

	pdir=opendir(bamDirectory.c_str()); //"." refers to the current dir
	
	if (!pdir){
		cout<<"opendir() failure; terminating"<<endl;
		exit(1);
	}
	else{
		cout<<"opendir() succeeded! Woo hoo!"<<endl;
		
	}
	errno=0;
	
	while ((pent=readdir(pdir))){
		string thisFileName=pent->d_name;
		int len=thisFileName.length();
		if(len>4 && thisFileName.substr(len-4, 4)==".bam" )
		{
			
			string fullFileName=bamDirectory+"/"+thisFileName;
			cout<<"bam file: "<<fullFileName<<endl;
			bamFileNames.push_back(fullFileName);	
		}
		//printf("%s", pent->d_name);
	}
 
	if (errno){
		printf ("readdir() failure; terminating");
		exit(1);
	}
	
	closedir(pdir);
}

void processBam( string bamFileName )
{
	Handy h(0);
	BamReader reader;
	BamAlignment al;
	RefVector refVector;
	
	string outputFilename;
	string outFileNameStub;
	
	outFileNameStub=h.getFileNameStubFromFileName(h.getFileNameFromPath(bamFileName));
	
	
	outputFilename=outDirectory+"/"+outFileNameStub+".umis_marked.bam";
	cout<<"Out file is: "<<outputFilename<<endl;
		
	int ch=0;
	
	if ( !reader.Open(bamFileName) ) 
	{
		cerr << "ERROR: Could not open input BAM file " <<bamFileName<< endl;
		return;
	}
	
	const SamHeader header = reader.GetHeader();
	const RefVector references = reader.GetReferenceData();

	
	// attempt to open our BamWriter
	BamWriter writer;
	writer.Open(outputFilename, header, references);
	
	if ( !writer.Open(outputFilename, header, references) ) {
		cerr << "Could not open output BAM file" <<outputFilename<< endl;
		return;
	}
	
	int i=0;
		
	while(reader.GetNextAlignment(al))
	{	
		doubleString UMI=umis[al.Name];
		
		al.AddTag("QX", "Z", UMI.s2);		
		al.AddTag("RX", "Z", UMI.s1);
		writer.SaveAlignment(al);
	}
	
	
	// close the reader & writer
	reader.Close();
	writer.Close();
}


/*===========================================================================================
Check that all of the necessary fields exist.
===========================================================================================*/

unsigned int checkErrors()
{
	//Errors 

	int err=0;
	Handy h(0);
	string problems="";
	
	
	
	if(outDirectory.length()==0)
	{
		problems.append("A stub for an outfile is needed (-out).\n");
		++err;
	}

	if(FASTQFileName.length()==0)
	{
		problems.append("A stub for an outfile is needed (-out).\n");
		++err;
	}	
	
		
	/*============================================================
	Check that all the relevant files can be read/written to
	==============================================================*/

	if(err>0)
	{		
		displayHelp();
	}

	return err;
	
}


void displayHelp()
{

	cout<<"This takes in the original FASTQ and an aligned bam and makes a new BAM file with the UMI marked.\n";
	cout<<"For example: ./AddUMIsToBam -fastq testData/test.fastq -bam testData/test.bam -out_dir testData -start_pos 7 -umi_length 3 \n";

	cout<<"Required options:\n";	
	cout<<"-fastq name of original FASTQ file which still contains the UMIs.\n";
	cout<<"-bam Name of the bam file. You can add more than one bam file e.g. -bam aligment1.bam -bam alignment2.bam (required). This assumes bam was based on trimmed FASTQ.\n";
	cout<<"-out_dir Name of the output directory to be used for the output. It will call the filename outputDir/alignment1.umis_marked.bam  \n";
	cout<<"-start_pos the position to start the UMI at. The first base in the read is pos 1. Default=1";
	cout<<"-umi_length the number of bases in the UMI. Default =8.";
	cout<<"\n\nThe first step of this program save the UMI and read name into memory. Thus, segmentation faults will occur if you load in a FASTQ file that is a lot bigger than the memory you allocate for the program.\n\n";
}


void somethingsGoneWrong(string whatsGoneWrong)
{

	cout<<"ERROR: Oh noooooooooooooo! Something has gone horribly wrong.\n";	
	cout<<whatsGoneWrong;
	cout<<"\n";	
	cerr<<"\nPlease try again.";
	
}
