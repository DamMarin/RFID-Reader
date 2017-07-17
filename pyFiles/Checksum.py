import re

def check_csum(sentence):

    checksum = sentence[len(sentence) - 2:]
    #print checksum
    
    # mporei kai na min xriazete auto. Stin ousia etoimazei thn protash
    #chksumdata = re.sub("(\n|\r\n)","", sentence[sentence.find("$")+1:sentence.find("*")])
    chksumdata = sentence[sentence.find("$"):sentence.find("*")]
    print chksumdata    

    csum = 0  
    for i in chksumdata:
        csum = ord(i)^csum 
    print csum
        
    if hex(csum) == hex(int(checksum)):
        check = True
    else:
        check = False

    return check;

#sentence='$RFTG;A01;0123456789*70'
#check=check_csum(sentence)
#print 'Check :' , check


def make_csum(sentence): # Calculate a Checksum
    sentence = sentence[sentence.find("$"):] #an balw +1 den ipologizei to $ 8elei skepsh
    #print sentence

    csum = 0
    for i in sentence:
        csum = ord(i)^csum
    #print 'CheckSum :' , csum

    return csum;

#sentence='$RFTG;A01;0123456789'
#csum=make_csum(sentence)
#print 'CheckSum :' , csum
