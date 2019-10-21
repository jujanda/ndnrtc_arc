import sys
import fileinput
import os

# initialise variables
data = []
settingNumber = "[PLACEHOLDER]"
runNumber = "[PLACEHOLDER]"
adaption = "[PLACEHOLDER]"
shapingProfile = "[PLACEHOLDER]"
consBandwith = "[PLACEHOLDER]"
framesMissing = "[PLACEHOLDER]"
psnr = "[PLACEHOLDER]"
ssim = "[PLACEHOLDER]"
vmaf = "[PLACEHOLDER]"

# save provided file arguments
PATH = sys.argv[1]

# get list of all "setting_x" folders
settingList = [ elem for elem in os.listdir(PATH) if "setting_" in elem]
# print settingList

for setting in settingList:
    settingNumber = setting.split("_")[-1]

    with open(PATH + setting + "/parameters.txt", "r") as file:
        lines = file.readlines()
        lineEntries = lines[0].split("\t")
        # runNumber = lineEntries[0]
        adaption = lineEntries[2]
        shapingProfile = lineEntries[3]
        consBandwith = lineEntries[4].strip()

    runList = [ elem for elem in os.listdir(PATH + setting + "/results/") if "." not in elem]
    # print runList

    for run in runList:
        runNumber = run

        with open(PATH + setting + "/results/" + run + "/arc_PostProcess.log","r") as file:
            lines = file.readlines()
            for line in lines:

                # Total frames missing: 133
                if "Total frames missing:" in line: 
                    framesMissing = line.split(" ")[-1].strip()

                # [Parsed_psnr_0 @ 0xba7f80] PSNR y:17.85 u:30.50 v:35.24 average:19.50 min:4.36 max:48.23
                elif "Parsed_psnr" in line:
                    psnr = line.split(" ")[-3].split(":")[-1]

                # [Parsed_ssim_0 @ 0xe87f80] SSIM Y:0.803438 U:0.921690 V:0.947602 All:0.847174 (8.158021)
                elif "Parsed_ssim" in line:
                    ssim = line.split(" ")[-2].split(":")[-1]

                # "VMAF_score": 38.427998305557658, 
                elif "VMAF_score" in line:
                    vmaf = line.split(" ")[-2][:-1]
                    # print "vmaf = "  + vmaf

        # construct result entry
        resultEntry = settingNumber + "\t" + runNumber + "\t" + adaption + "\t" + shapingProfile + "\t" + consBandwith + "\t" + framesMissing + "\t" + psnr + "\t" + ssim + "\t" + vmaf + "\n"

        # print resultEntry

        # add line to data
        data.append(resultEntry)

# write data to file
with open(PATH + "run_overview.csv", 'w') as file:
    file.writelines(data)
