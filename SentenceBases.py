import Checksum 
import Output
from time import strftime

hhmmss=strftime("%H%M%S")
dd=strftime("%d")
mm=strftime("%m")
yyyy=strftime("%Y")

def Base(message):
	check = Checksum.check_csum(message)
	if check == True :
		mystr=message.split(';') #split() split by ';'
		#print mystr
		base = mystr[0][1:]
		if base == "RFTG":
			access = RFTG(message)
			if access:
				final = "ACCEPT"  #RFDT()
			else:
				final = "DENY"
		# if base == "RF??":
			# RF??(message)
		
	return final;

################### RFDT ###################
#
#   $RFDT;A01;081530;10;02;2016*60
#   $RFDT;reader_id,hhmmss;dd;mm;yyyy*CC
# where:
#	 	hh,mm,ss   Hr,Min,Sec
#       dd,mm,yyyy Day,Month,Year
#       *CC        Checksum
#
############################################
#reader_id='A05' #dokimastikh timh
def RFDT():
    reader_id='A05' #dokimastikh timh
    sentence='$RFDT;{};{};{};{};{}'.format(reader_id,hhmmss,dd,mm,yyyy)
    #print (sentence)

    csum= Checksum.make_csum(sentence) # Calculate a Checksum (from other .py file)
    #print csum

    sentence='{}*{}'.format(sentence,csum)
    #print sentence
    return sentence; #anti gia RFDT()

##################################
#Tin kalw apo allo arxeio etsi:  #
#import SentenceBases            #
#sentence=SentenceBases.RFDT()   #
#print sentence                  #
##################################

#----------------------------------------------------------------

################### RFTG ###################
#
#       $RFTG;A01;0123456789*98
#       $RFTG;reader;tag*CC
# where:
#	 	reader     Reader id
#       tag        Tag id
#       *CC        Checksum
#
############################################
def RFTG(message):
	mystr=message.split(';') #split() split by ';'
	reader = mystr[1]
	tag = mystr[2]
	mystr2=tag.split('*')
	#print mystr2
	tag=mystr2[0]

	print tag
	message = '{};{}'.format(reader,tag)
	access = Output.searchfile(tag)
	if access == True:
		Output.savefile(message)  #############na bgalw to # xreiazete
		#sentence = RFDT()	
	return access;

