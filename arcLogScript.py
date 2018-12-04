import matplotlib
import matplotlib.pyplot as plt
import numpy as np


def read_file(filename):
	names = []
	file = open(filename, "r")

	# exract reference time from first line
	ref_time = int(file.readline().strip().split("\t")[0])

	# go through every line in the file except the first
	for line in file.readlines():

		# remove whitespace characters and split line at tabs
		record = line.strip().split("\t")
		
		try:
			# transform absolute time into relative simulation time
   			time = int(record[0]) - ref_time

   			# save data in list
			names.append([time, record.pop()])

		except ValueError:
   			print "ValueError: " + record[0] + " cant be converted."
		
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
		fh.write(str(record[0]) + "\t" + record[1] + "\n")

	fh.close() 

def find_missing_packets(recordList, nameList):
	lost = []
	for record in recordList:
		if record[1] not in nameList:
			lost.append(record)
	return lost

# ===================================

# workaround for logger bug
with open("/tmp/arcLog_producerReceivedInterests.csv","r") as input:
    with open("/tmp/arcLog_producerReceivedInterests_fixed.csv","wb") as output: 
        file = input.readlines()
        del file[1]
        for line in file:
            output.write(line)    

# open files
consSenInt = read_file("/tmp/arcLog_consumerSentInterests.csv") 
prodRecInt = read_file("/tmp/arcLog_producerReceivedInterests_fixed.csv") 
prodSenDat = read_file("/tmp/arcLog_producerSentData.csv") 
consRecDat = read_file("/tmp/arcLog_consumerReceivedData.csv")
threadSwitches = read_file("/tmp/arcLog_threadswitches.csv")
lists = [consSenInt, prodRecInt, prodSenDat, consRecDat]


# # delete the reference time from each of the name lists
# for namelist in lists:
# 	del namelist[0]

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
# print "===== # of missing segments per list ====="
# print len(losslist01)
# print len(losslist02)
# print len(losslist03)

# Data for plotting
# t = np.arange(0.0, 2.0, 0.01)
# s = 1 + np.sin(2 * np.pi * t)

# fig, ax = plt.subplots()
# ax.plot(t, s)

# ax.set(xlabel='time (s)', ylabel='voltage (mV)',
#        title='About as simple as it gets, folks')
# ax.grid()

# fig.savefig("/tmp/test.png")
# plt.show()

x = []
for record in consSenInt:
	x.append(record[0])

y = []
for record in losslist03:
	y.append(record[0])

switches = []
threads = []
for record in threadSwitches:
	switches.append(record[0])
	threads.append(record[1])

# Generate a normal distribution, center at x=0 and y=5
fig, ax = plt.subplots(1,2)

# We can set the number of bins with the `bins` kwarg
ax[0].hist(x, bins=300)
ax[1].hist(y, bins=300)

ax[0].set_xlim(0, 30000)
ax[0].set_ylim(0, 200)
ax[1].set_xlim(0, 30000)
ax[1].set_ylim(0, 200)

ax[0].vlines(switches, 0, 1, transform=ax[0].get_xaxis_transform(), colors='r')
ax[1].vlines(switches, 0, 1, transform=ax[1].get_xaxis_transform(), colors='r')
x_bounds_0 = ax[0].get_xlim()
x_bounds_1 = ax[0].get_xlim()

i = 0
while i<len(threads):
	ax[0].annotate(s=threads[i], xy =(((switches[i]-x_bounds_0[0])/(x_bounds_0[1]-x_bounds_0[0])),1.01), xycoords='axes fraction', verticalalignment='right', horizontalalignment='right bottom' , rotation = 270)
	ax[1].annotate(s=threads[i], xy =(((switches[i]-x_bounds_1[0])/(x_bounds_1[1]-x_bounds_1[0])),1.01), xycoords='axes fraction', verticalalignment='right', horizontalalignment='right bottom' , rotation = 270)
	i+=1

fig.savefig("/tmp/arcLog_hist.png")
plt.show()




