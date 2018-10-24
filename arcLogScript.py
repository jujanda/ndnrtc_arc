
def read_file(filename):
	names = []
	file = open(filename, "r")

	# go through every line in the file except the first (1 to end instead of 0 to end)
	for line in file.readlines():

		# remove whitespace characters and split line at tabs
		record = line.strip().split("\t")
		names.append([record[0], record.pop()])

	file.close()
	return names

def get_names_only(listname):
	namelist = []
	for record in listname:
		namelist.append(record[1])

	return namelist


def write_list_to_file(listname, filename):
	fh = open(filename, "w") 

	# ???
	for record in listname:
		time = int(record[0]) - ref_time
		fh.write(str(time) + "\t" + record[1] + "\n")

	fh.close() 

def find_missing_packets(recordList, nameList):
	lost = []
	for record in recordList:
		if record[1] not in nameList:
			lost.append(record)
	return lost

# ===================================

ref_time = 0

# open files
consSenInt = read_file("/tmp/arcLog_consumerSentInterests.csv") 
prodRecInt = read_file("/tmp/arcLog_producerReceivedInterests.csv") 
prodSenDat = read_file("/tmp/arcLog_producerSentData.csv") 
consRecDat = read_file("/tmp/arcLog_consumerReceivedData.csv") 
lists = [consSenInt, prodRecInt, prodSenDat, consRecDat]

# workaround for logger bug
del prodRecInt[1]

# save reference time in own variable
ref_time = int(lists[0][0].pop())

# delete the reference time from each of the name lists
for namelist in lists:
	del namelist[0]

consSenInt_names = get_names_only(consSenInt)
prodRecInt_names = get_names_only(prodRecInt)
prodSenDat_names = get_names_only(prodSenDat)
consRecDat_names = get_names_only(consRecDat)

# find which packets went missing
# TODO: eliminate false-positives in losslist2 (due to meta data after names of manifest packets) 
losslist01 = find_missing_packets(consSenInt, prodRecInt_names)
losslist02 = find_missing_packets(prodRecInt, prodSenDat_names)
losslist03 = find_missing_packets(consSenInt, consRecDat_names)

# write results to files
write_list_to_file(losslist01, "/tmp/arcLog_producerMissingInterests.csv")
write_list_to_file(losslist02, "/tmp/arcLog_producerMissingData.csv")
write_list_to_file(losslist03, "/tmp/arcLog_consumerMissingData.csv")



# print some results to console for debugging
print "===== # of Interest/Data per list ====="
for namelist in lists:
	print len(namelist)
# print "===== Differences between lists ====="
# print len(set(consSenInt_names) - (set(prodRecInt_names)))
# print len(set(prodRecInt_names) - (set(prodSenDat_names)))
# print len(set(prodSenDat_names) - (set(consRecDat_names)))
# print len(set(consSenInt_names) - (set(consRecDat_names)))
# print "===== # of missing Data for consumer (without duplicates) ====="
# print len(set(prodRecInt_names) - (set(consSenInt_names)))
print "===== # of missing segments per list ====="
print len(losslist01)
print len(losslist02)
print len(losslist03)


