from rinex_python import RinexPython
import copy
import datetime
import os

# Create a RinexPython object
inst = RinexPython()

# Declare blank array of ephemerides
ephems = []


def ephemIsNewer(ephem1, ephem2):
    """ return True if ephem1 > ephem2"""
    if ephem1['TocYear'] > ephem2['TocYear']:
        return True
    elif ephem1['TocYear'] < ephem2['TocYear']:
        return False

    if ephem1['TocMonth'] > ephem2['TocMonth']:
        return True
    elif ephem1['TocMonth'] < ephem2['TocMonth']:
        return False

    if ephem1['TocYear'] > ephem2['TocYear']:
        return True
    elif ephem1['TocYear'] < ephem2['TocYear']:
        return False

    if ephem1['TocDay'] > ephem2['TocDay']:
        return True
    elif ephem1['TocDay'] < ephem2['TocDay']:
        return False

    if ephem1['TocHr'] > ephem2['TocHr']:
        return True
    elif ephem1['TocHr'] < ephem2['TocHr']:
        return False

    if ephem1['TocMin'] > ephem2['TocMin']:
        return True
    elif ephem1['TocMin'] < ephem2['TocMin']:
        return False

    if ephem1['TocSec'] > ephem2['TocSec']:
        return True

    return False

def generateEphemerides(dir, ephemDateTime):
    for file in os.listdir(os.fsencode(dir)):
        filename = os.fsdecode(file)
        if filename.endswith("n") or filename.endswith(".nav"):
            # Parse ephemeride file
            inst.parseFromFile(os.path.join(dir, filename))

            # Get the ephemerides
            origEphems = inst.getEphemerides()

            # Loop over all PRNs
            for prn in range(1, 33):

                # Loop over all ephemerides
                for ephem in origEphems:
                    # See if this ephemeris is newer than the one we currently have
                    if ephem['PRN'] == prn:
                        found = False
                        for idx in range(len(ephems)):
                            if ephems[idx]['PRN'] == prn:
                                found = True
                                # Is this a newer ephemeride?
                                if ephemIsNewer(ephem, ephems[idx]):
                                    ephems[idx] = ephem
                        # If an ephemeride was not found then append a new one
                        if not found:
                            ephems.append(ephem)

    # Declare blank array of new ephemerides
    newEphems = []

    # We now need to loop over days
    for day in range(0, 24):
        # Loop over all hours
        for hour in range(0, 24, 2):
            # Loop over all PRNs
            for ephem in ephems:
                # Generate the new ephemeris
                #newDateTime = inst.getEphemerisDateTime(ephem) + datetime.timedelta(days=day, hours=hour)
                newDateTime = ephemDateTime + datetime.timedelta(days=day, hours=hour)
                newEphem = inst.changeEphemerisTime(ephem, newDateTime)
                # Add the new ephemeris to the array
                newEphems.append(newEphem)

    # Now update the ephemerides in the instance
    inst.contents['Ephemerides'] = newEphems

    # Write the contents to a file
    newfilename = "{}.nav".format(ephemDateTime.strftime("%Y%m%dT%H-%M-%S"))
    fileandpath = os.path.join("/tmp", newfilename)
    inst.writeToFile(fileandpath)
    return os.path.abspath(fileandpath)


if __name__ == "__main__":
    print(generateEphemerides("tests", datetime.datetime.now()))
