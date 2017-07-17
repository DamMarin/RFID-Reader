import sys
import os
import csv
from time import strftime

Date = strftime("%Y_%m_%d")
Time = strftime("%H:%M;")
#Time = strftime("%H:%M:%S")
path = r'C:\Python_server'

def savelog(log):

    try:
        if not os.path.exists(path):
            os.makedirs(path) 
        filename = path+'/'+Date+"_log.csv"
        csvfile = open(filename,'a')
        # If the file does not exists, open(name,'r+') will fail.
        # 'w' : which creates the file if the file does not exist, but it will truncate the existing file.
        # 'a' : this will create the file if the file does not exist, but will not truncate the existing file.
        # to avoid modifying an existing file, 'x' could be used instead of 'w', 'afieldnames = ['first_name', 'last_name','am']
        csvfile.close()
        
        with open(filename, 'ab') as csvfile:
            fieldnames = ['Time', 'Log']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            #writer.writeheader() #writing the fieldnames in header
            writer.writerow({'Time': Time, 'Log': log})
            
    except:
            print("error occured in savelog")
            sys.exit(0)         # quit Python        

    return log;


def savefile(data):

    try:
        if not os.path.exists(path):
            os.makedirs(path) 
        filename = path+'/'+Date+".csv"
        csvfile = open(filename,'a')
        csvfile.close()
        
        with open(filename, 'ab') as csvfile:
            fieldnames = ['Time', 'Data']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            #writer.writeheader() #writing the fieldnames in header
            writer.writerow({'Time': Time, 'Data': data})
            
    except:
            print("error occured in savefile")
            sys.exit(0)         # quit Python        

    return data;

def searchfile(tag):
	try:
		filename = path+'/'+"Users.csv"
						
		with open(filename, 'rt') as csvfile:
			reader = csv.reader(csvfile,delimiter=',')
			count = 0
			for row in reader:
				for field in row:
					if field == tag:
						count += 1
						access = True
						print 'ACCEPT'
						
			if count == 0:
				access = False
				print 'DENY'
				
	except:
		print("error occured in search_user")
		#sys.exit(0)         # quit Python    
	return access;
