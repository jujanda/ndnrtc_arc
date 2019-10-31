import sys
import fileinput
import os
import numpy as np

# Extract number from setting string
def getSettingNumber(tmp):
    return int(tmp.split("_")[-1])

# Initialise variables
data = []
data_means = []
settingNumber = "[PLACEHOLDER]"
runNumber = "[PLACEHOLDER]"
initialResolution = "[PLACEHOLDER]"
adaption = "[PLACEHOLDER]"
shapingProfile = "[PLACEHOLDER]"
consBandwith = "[PLACEHOLDER]"
framesMissing = "[PLACEHOLDER]"
retransmissions_total = "[PLACEHOLDER]"
psnr = "[PLACEHOLDER]"
ssim = "[PLACEHOLDER]"
vmaf = "[PLACEHOLDER]"


# Save provided file arguments
PATH = sys.argv[1]

# Get list of all "setting_x" folders
settingList = [ elem for elem in os.listdir(PATH) if "setting_" in elem]
settingList.sort(key=getSettingNumber)

# Built & append headlines
headline = "Setting" + "\t" + "Run" + "\t" + "Init._Res." + "\t" + "Adaption_Logic" + "\t" + "Shaping_Profile" + "\t" + "Bandwith(cons)" + "\t" + "Frames_Missing" + "\t" + "Retr._max" + "\t" + "Retr._total" + "\t" + "PSNR" + "\t" + "SSIM" + "\t" + "VMAF" + "\n"
headline_means = "Setting" + "\t" + "Run" + "\t" + "Init._Res." + "\t" + "Adaption_Logic" + "\t" + "Shaping_Profile" + "\t" + "Bandwith(cons)" + "\t" + "Frames_Missing"+ "\t" + "Retransmissions" + "\t" + "PSNR" + "\t" + "SSIM" + "\t" + "VMAF" + "\n"
data.append(headline)
data_means.append(headline_means)

print "> Parsing files"

for setting in settingList:

    # Prepare to store values
    framesMissing_values = []
    retransmissions_values = []
    psnr_values = []
    ssim_values = []
    vmaf_values = []

    # Parse setting number
    settingNumber = setting.split("_")[-1]

    # Parse setting values
    with open(PATH + setting + "/parameters.txt", "r") as file:
        lines = file.readlines()
        lineEntries = lines[0].split("\t")
        # runNumber = lineEntries[0]
        initialResolution = lineEntries[1]
        shapingProfile = lineEntries[2]
        consBandwith = lineEntries[3].strip()

    # Get list of all run folders
    runList = [ elem for elem in os.listdir(PATH + setting + "/results/") if "." not in elem]

    for run in runList:

        # Prepare variables
        runNumber = run
        retransmission_tmp = []

        # Parse run values
        with open(PATH + setting + "/results/" + run + "/arcLog_retransmissions.csv","r") as file:
            lines = file.readlines()
            retransmissions_total = str(len(lines)-1)

        # Parse run values
        with open(PATH + setting + "/results/" + run + "/src/remote-stream-impl.cpp","r") as file:
            for line in file.readlines():

                # arc_ = make_shared<Arc>(AdaptionLogic::NoAdaption, this, sstorage_);
                if "AdaptionLogic::" in line:
                    adaption = line.split("::")[-1].split(",")[0]

        # Parse run values
        with open(PATH + setting + "/results/" + run + "/arc_PostProcess.log","r") as file:
            for line in file.readlines():

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

        # Parse run values
        with open(PATH + setting + "/results/" + run + "/arcLog_networkMeasurements.csv","r") as file:
            # Skip first line
            for line in file.readlines()[1:]:
                # 1572030662419   [INFO ]: [measured] retransmissions = 103, oldThroughput = 754.379 kBit/s, newThroughput = 103 kBit/s
                retransmission_tmp.append(int(line.split("=")[1].split(",")[0].strip()))

        # Construct result entry
        resultEntry = []
        resultEntry.append(settingNumber + "\t") 
        resultEntry.append(runNumber + "\t") 
        resultEntry.append(initialResolution + "\t") 
        resultEntry.append(adaption + "\t") 
        resultEntry.append(shapingProfile + "\t") 
        resultEntry.append(consBandwith + "\t") 
        resultEntry.append(framesMissing + "\t") 
        resultEntry.append(str(max(retransmission_tmp)) + "\t")
        resultEntry.append(retransmissions_total + "\t") 
        resultEntry.append(psnr + "\t") 
        resultEntry.append(ssim + "\t") 
        resultEntry.append(vmaf + "\n")

        # Add line to data
        data.append("".join(resultEntry))

        # Save values for summary calulations
        retransmissions_values.append(int(retransmissions_total))
        framesMissing_values.append(int(framesMissing))
        psnr_values.append(float(psnr))
        ssim_values.append(float(ssim))
        vmaf_values.append(float(vmaf))

    # Construct summary entry
    summary = []
    summary.append(settingNumber + "\t") 
    summary.append(str(len(runList)) + "\t") 
    summary.append(initialResolution + "\t") 
    summary.append(adaption + "\t") 
    summary.append(shapingProfile + "\t") 
    summary.append(consBandwith + "\t") 
    summary.append(str(np.mean(np.asarray(framesMissing_values))) + "\t")
    summary.append(str(np.mean(np.asarray(retransmissions_values))) + "\t")
    summary.append(str(np.mean(np.asarray(psnr_values))) + "\t")
    summary.append(str(np.mean(np.asarray(ssim_values))) + "\t")
    summary.append(str(np.mean(np.asarray(vmaf_values))) + "\n")
    data_means.append("".join(summary))

print "> Saving results"

# Write normal data to file
with open(PATH + "run_overview.csv", 'w') as file:
    file.writelines(data)

# Write summary data to file
with open(PATH + "run_overview_means.csv", 'w') as file:
    file.writelines(data_means)