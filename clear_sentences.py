##############################################################
#
#   Diabazei apo to buffer kai katharizei tis protaseis
#   to ipolipo tou buffer prostithete sto epomeno
#
##############################################################

from Output import savefile
from SentenceBases import Base

def clear(data):
	
	print 'Data :',data
	countD = data.count('$') #counting the character '$'
	countS = data.count('*') #counting the character '*'
	Dollar = data.find("$")	 # to .find epistrefei tin timi -1 ean den brei ton Character (se antithesi me tin .index)
	Star = data.find("*")
	Dnext = data.find('$',Dollar+1)
	Snext = data.find("*",Star+1)
	print'Dollar:',Dollar,'|  Star',Star,'|  Dnext',Dnext,'|  Snext',Snext

	if countD>=countS:
		count=countD
	else:
		count=countS
	print 'count:',count,'|  countD:',countD,'|  countS:',countS
	

	if countD!=0 and countS!=0 :   # if $ does not exist erase the data	
		if Dnext > Star  or (Dnext == -1):
			if (Star >= 0) & (Dollar >= 0) :
				msg=data[Dollar:Star+3]  
				print 'MESSAGE :',msg
				if msg is not '' :
					message=msg
					#savefile(msg) #######################################
		else:
			if (Star >= 0) & (Dollar >= 0) :
				msg=data[Dnext:Star+3]  
				print 'MESSAGE :',msg
				if msg is not '' :
					message=msg
					#savefile(msg) #######################################
	return message;

	
	
