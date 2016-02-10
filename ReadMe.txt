AddUMIsToBam
Takes:
A FASTQ file
A bam file
Adds the UMI and UMI base qualities to the bam file in the RX and QX tags

Developed by: Michele Busby, Computational Biologist Broad Technology Labs/MBRD Broad Institute 2/10/2016

Why would I want to do this?

The UMIs are usually contained in the original FASTQ and then trimmed off prior alignment. The aligner doesn't carry these bases through to the output bam alignment file. But they often needed in downstream analyses. Putting them in the RX and QX tags allows you to look at them during downstream analysis. 

The RX and QX conventions will probably soon be implemented in Broad sequencing platform standard processes. This convention is supported in the picard tools duplicate marking.

In our processes a single original FASTQ may be split into multiple samples using an in line barcode, resulting in many bams corresponding to each original FASTQ. For this reason, we support associating multiple bams with a single FASTQ, i.e. 
-fastq file.fastq -bam bam1.bam -bam bam2.bam -bam bam3.bam 

This is a one to many relationship - it assumes every read in the bam will be in the FASTQ but not that every read in the FASTQ will be in the bam.

The output is a new bam file 

*Useage:

./AddUMIsToBam -fastq testData/test.fastq -bam testData/test.bam -out_dir testData -start_pos 7 -umi_length 3 

-fastq name of original FASTQ file which still contains the UMIs. (required)

-bam Name of the bam file. You can add more than one bam file e.g. -bam aligment1.bam -bam alignment2.bam This assumes bam was based on trimmed FASTQ. (required)

-out_dir Name of the output directory to be used for the output. It will call the filename outputDir/alignment_name.umis_marked.bam where alignment_name is the file name of the original file. If you input a bunch of bams with the new same file name odd things will happen, none of them what you want. (required)

-start_pos the position to start the UMI at. The first base in the read is pos 1. Default=1-umi_length the number of bases in the UMI. Default =8.

The program works by creating a hash map of all of the read names and all of the UMIs in memory. It might segment fault if you have more read reads than memory available so make sure you give it enough memory.

Bamtools

To get this to compile you will need to download Derek Barnett's bamtools API and install it in a folder somewhere. It is here: https://github.com/pezmaster31/bamtools You need the API, not the command line program though is quite useful to have. Then, after you install it, you need to edit the Makefile in this folder so that everywhere it says "FolderWhereBamToolsIs" you put in the folder where bamtools is located.

Compiling

Go to the folder where everything is installed in type "make". Ignore the warnings about Handy doing stupid stuff. If nothing says ERROR and it makes an executable called AddUMIsToBam you should be all set.

