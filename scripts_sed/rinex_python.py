import logging
import io
import re
from datetime import datetime
from datetime import timedelta
import math
import copy

# Define an error class that we can throw on error
class ParseError(Exception):
    pass

# Class to handle the reading/writing of RINEX files
class RinexPython:

    # Constructor
    def __init__(self):

        logging.debug("Constructor called")

        # Initialise the class member variable that will store the rinex file contents
        self.contents = {}
        self.contents['Header'] = {}
        self.contents['Ephemerides'] = []

    # Override the equals operator
    def __eq__(self, other):
        if isinstance(other, RinexPython):
            return self.contents == other.contents
        else:
            return False

    # Method to fix the floating point Exp designator. 'D' is an allowed
    # character (apparently for legacy compiler support)
    def _fixExpDesignator(self, inputString):
        return inputString.replace("D", "E")
    
    # Method to unwrap angles (radians). If vaulue is greater than +/- pi,
    # the subtract/add multiples of 2*pi to bring within range
    def _unwrapAngle(self, angle):
        while (abs(angle) > math.pi):
            angle -= math.copysign(1,angle)*2*math.pi
        return angle
        


    # Method to parse the header
    #  +----------------------------------------------------------------------------+
    #  |                                   TABLE A3                                 |
    #  |         GPS NAVIGATION MESSAGE FILE - HEADER SECTION DESCRIPTION           |
    #  +--------------------+------------------------------------------+------------+
    #  |    HEADER LABEL    |               DESCRIPTION                |   FORMAT   |
    #  |  (Columns 61-80)   |                                          |            |
    #  +--------------------+------------------------------------------+------------+
    #  |RINEX VERSION / TYPE| - Format version (2.11)                  | F9.2,11X,  |
    #  |                    | - File type ('N' for Navigation data)    |   A1,19X   |
    #  +--------------------+------------------------------------------+------------+
    #  |PGM / RUN BY / DATE | - Name of program creating current file  |     A20,   |
    #  |                    | - Name of agency  creating current file  |     A20,   |
    #  |                    | - Date of file creation                  |     A20    |
    #  +--------------------+------------------------------------------+------------+
    # *|COMMENT             | Comment line(s)                          |     A60    |*
    #  +--------------------+------------------------------------------+------------+
    # *|ION ALPHA           | Ionosphere parameters A0-A3 of almanac   |  2X,4D12.4 |*
    #  |                    | (page 18 of subframe 4)                  |            |
    #  +--------------------+------------------------------------------+------------+
    # *|ION BETA            | Ionosphere parameters B0-B3 of almanac   |  2X,4D12.4 |*
    #  +--------------------+------------------------------------------+------------+
    # *|DELTA-UTC: A0,A1,T,W| Almanac parameters to compute time in UTC| 3X,2D19.12,|*
    #  |                    | (page 18 of subframe 4)                  |     2I9    |
    #  |                    | A0,A1: terms of polynomial               |            |
    #  |                    | T    : reference time for UTC data       |      *)    |
    #  |                    | W    : UTC reference week number.        |            |
    #  |                    |        Continuous number, not mod(1024)! |            |
    #  +--------------------+------------------------------------------+------------+
    # *|LEAP SECONDS        | Delta time due to leap seconds           |     I6     |*
    #  +--------------------+------------------------------------------+------------+
    #  |END OF HEADER       | Last record in the header section.       |    60X     |
    #  +--------------------+------------------------------------------+------------+
    def _parseHeader(self, header):
        logging.debug("_parseHeader called")

        # Work through the header and pull out only the compulsory fields
        for line in header :
            # Search for version / type
            z = re.match("^(.{9}).{11}(.{20}).*RINEX VERSION / TYPE", line)
            if z != None :
                # Found a match, so extract the fields
                self.contents['Header']['Version']  = float(z.group(1))
                # Check the version we are parsing
                if self.contents['Header']['Version'] != 2.0 :
                    raise ParseError(f"Error only RINEX v2.0 supported. Detected RINEX v{self.contents['Header']['Version']}")

                self.contents['Header']['Type']     = z.group(2)
                 

            # Search for pgm/run by/date
            z = re.match("^(.{20})(.{20})(.{20})PGM / RUN BY / DATE", line)
            if z != None :
                # Found a match, so extract the fields
                self.contents['Header']['Program']  = z.group(1)
                self.contents['Header']['RunBy']    = z.group(2)
                self.contents['Header']['Date']     = z.group(3)

        return 
    
    # Method to parse the first ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  |PRN / EPOCH / SV CLK| - Satellite PRN number                   |     I2,    |
    #  |                    | - Epoch: Toc - Time of Clock             |            |
    #  |                    |          year         (2 digits)         |    5I3,    |
    #  |                    |          month                           |            |
    #  |                    |          day                             |            |
    #  |                    |          hour                            |            |
    #  |                    |          minute                          |            |
    #  |                    |          second                          |    F5.1,   |
    #  |                    | - SV clock bias       (seconds)          |  3D19.12   |
    #  |                    | - SV clock drift      (sec/sec)          |            |
    #  |                    | - SV clock drift rate (sec/sec2)         |            |
    #  +--------------------+------------------------------------------+------------+
    def _parseEphemerisLine1(self, line, parsedLine):
        z = re.match("(.{2})(.{3})(.{3})(.{3})(.{3})(.{3})(.{5})(.{19})(.{19})(.{19})", line)
        if z == None :
            raise ParseError("Error parsing line 1 of ephemeris")
        
        # Extract the contents
        parsedLine['PRN']               = int(z.group(1))
        parsedLine['TocYear']           = int(z.group(2))
        parsedLine['TocMonth']          = int(z.group(3))
        parsedLine['TocDay']            = int(z.group(4))
        parsedLine['TocHr']             = int(z.group(5))
        parsedLine['TocMin']            = int(z.group(6))
        parsedLine['TocSec']            = float(self._fixExpDesignator(z.group(7)))
        parsedLine['ClockBias']         = float(self._fixExpDesignator(z.group(8)))
        parsedLine['ClockDrift']        = float(self._fixExpDesignator(z.group(9)))
        parsedLine['ClockDriftRate']    = float(self._fixExpDesignator(z.group(10)))
        return
    

    # Method to parse the 2nd ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  | BROADCAST ORBIT - 1| - IODE Issue of Data, Ephemeris          | 3X,4D19.12 |
    #  |                    | - Crs                 (meters)           |            |
    #  |                    | - Delta n             (radians/sec)      |            |
    #  |                    | - M0                  (radians)          |            |
    #  +--------------------+------------------------------------------+------------+
    def _parseEphemerisLine2(self, line, parsedLine):
        z = re.match(".{3}(.{19})(.{19})(.{19})(.{19})", line)
        if z == None :
            raise ParseError("Error parsing line 2 of ephemeris")
        
        # Extract the contents
        parsedLine['IODE']      = float(self._fixExpDesignator(z.group(1)))
        parsedLine['Crs']       = float(self._fixExpDesignator(z.group(2)))
        parsedLine['DeltaN']    = float(self._fixExpDesignator(z.group(3)))
        parsedLine['M0']        = float(self._fixExpDesignator(z.group(4)))
        return
    
    # Method to parse the 3rd ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  | BROADCAST ORBIT - 2| - Cuc                 (radians)          | 3X,4D19.12 |
    #  |                    | - e Eccentricity                         |            |
    #  |                    | - Cus                 (radians)          |            |
    #  |                    | - sqrt(A)             (sqrt(m))          |            |
    #  +--------------------+------------------------------------------+------------+
    def _parseEphemerisLine3(self, line, parsedLine):
        z = re.match(".{3}(.{19})(.{19})(.{19})(.{19})", line)
        if z == None :
            raise ParseError("Error parsing line 3 of ephemeris")
        
        # Extract the contents
        parsedLine['Cuc']       = float(self._fixExpDesignator(z.group(1)))
        parsedLine['e']         = float(self._fixExpDesignator(z.group(2)))
        parsedLine['Cus']       = float(self._fixExpDesignator(z.group(3)))
        parsedLine['SqrtA']     = float(self._fixExpDesignator(z.group(4)))
        return
    
    # Method to parse the 4th ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  | BROADCAST ORBIT - 3| - Toe Time of Ephemeris                  | 3X,4D19.12 |
    #  |                    |                       (sec of GPS week)  |            |
    #  |                    | - Cic                 (radians)          |            |
    #  |                    | - OMEGA               (radians)          |            |
    #  |                    | - CIS                 (radians)          |            |
    #  +--------------------+------------------------------------------+------------+
    def _parseEphemerisLine4(self, line, parsedLine):
        z = re.match(".{3}(.{19})(.{19})(.{19})(.{19})", line)
        if z == None :
            raise ParseError("Error parsing line 4 of ephemeris")
        
        # Extract the contents
        parsedLine['Toe']       = float(self._fixExpDesignator(z.group(1)))
        parsedLine['Cic']       = float(self._fixExpDesignator(z.group(2)))
        parsedLine['OMEGA_uc']  = float(self._fixExpDesignator(z.group(3)))
        parsedLine['Cis']       = float(self._fixExpDesignator(z.group(4)))
        return
    
    # Method to parse the 5th ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  | BROADCAST ORBIT - 4| - i0                  (radians)          | 3X,4D19.12 |
    #  |                    | - Crc                 (meters)           |            |
    #  |                    | - omega               (radians)          |            |
    #  |                    | - OMEGA DOT           (radians/sec)      |            |
    #  +--------------------+------------------------------------------+------------+    
    def _parseEphemerisLine5(self, line, parsedLine):
        z = re.match(".{3}(.{19})(.{19})(.{19})(.{19})", line)
        if z == None :
            raise ParseError("Error parsing line 5 of ephemeris")
        
        # Extract the contents
        parsedLine['i0']        = float(self._fixExpDesignator(z.group(1)))
        parsedLine['Crc']       = float(self._fixExpDesignator(z.group(2)))
        parsedLine['omega_lc']  = float(self._fixExpDesignator(z.group(3)))
        parsedLine['OMEGA_DOT'] = float(self._fixExpDesignator(z.group(4)))
        return
    
    # Method to parse the 6th ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  | BROADCAST ORBIT - 5| - IDOT                (radians/sec)      | 3X,4D19.12 |
    #  |                    | - Codes on L2 channel                    |            |
    #  |                    | - GPS Week # (to go with TOE)            |            |
    #  |                    |   Continuous number, not mod(1024)!      |            |
    #  |                    | - L2 P data flag                         |            |
    #  +--------------------+------------------------------------------+------------+
    def _parseEphemerisLine6(self, line, parsedLine):
        z = re.match(".{3}(.{19})(.{19})(.{19})(.{19})", line)
        if z == None :
            raise ParseError("Error parsing line 6 of ephemeris")
        
        # Extract the contents
        parsedLine['IDOT']      = float(self._fixExpDesignator(z.group(1)))
        parsedLine['L2_Codes']  = float(self._fixExpDesignator(z.group(2)))
        parsedLine['GpsWeek']   = float(self._fixExpDesignator(z.group(3)))
        parsedLine['L2_P']      = float(self._fixExpDesignator(z.group(4)))
        return
    
    # Method to parse the 7th ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  | BROADCAST ORBIT - 6| - SV accuracy         (meters)           | 3X,4D19.12 |
    #  |                    | - SV health           (MSB only)         |            |
    #  |                    | - TGD                 (seconds)          |            |
    #  |                    | - IODC Issue of Data, Clock              |            |
    #  +--------------------+------------------------------------------+------------+
    def _parseEphemerisLine7(self, line, parsedLine):
        z = re.match(".{3}(.{19})(.{19})(.{19})(.{19})", line)
        if z == None :
            raise ParseError("Error parsing line 7 of ephemeris")
        
        # Extract the contents
        parsedLine['SV_Accuracy']   = float(self._fixExpDesignator(z.group(1)))
        parsedLine['SV_Health']     = float(self._fixExpDesignator(z.group(2)))
        parsedLine['TGD']           = float(self._fixExpDesignator(z.group(3)))
        parsedLine['IODC']          = float(self._fixExpDesignator(z.group(4)))
        return
    
    # Method to parse the 8th ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  | BROADCAST ORBIT - 7| - Transmission time of message           | 3X,4D19.12 |
    #  |                    |         (sec of GPS week, derived e.g.   |            |
    #  |                    |    from Z-count in Hand Over Word (HOW)  |            |
    #  |                    | - spare                                  |            |
    #  |                    | - spare                                  |            |
    #  |                    | - spare                                  |            |
    #  +--------------------+------------------------------------------+------------+    
    def _parseEphemerisLine8(self, line, parsedLine):
        z = re.match(".{3}(.{19})", line)
        if z == None :
            raise ParseError("Error parsing line 8 of ephemeris")
        
        # Extract the contents
        parsedLine['TxTime']   = float(self._fixExpDesignator(z.group(1)))
        return
        

    # Method to parse an ephemeris
    def _parseEphemeris(self, ephemerisLines):
        # Variable for the ephemeris
        ephemeris = {}
        self._parseEphemerisLine1(ephemerisLines[0], ephemeris)
        self._parseEphemerisLine2(ephemerisLines[1], ephemeris)
        self._parseEphemerisLine3(ephemerisLines[2], ephemeris)
        self._parseEphemerisLine4(ephemerisLines[3], ephemeris)
        self._parseEphemerisLine5(ephemerisLines[4], ephemeris)
        self._parseEphemerisLine6(ephemerisLines[5], ephemeris)
        self._parseEphemerisLine7(ephemerisLines[6], ephemeris)
        self._parseEphemerisLine8(ephemerisLines[7], ephemeris)
        return ephemeris
        
        

    # Method to parse a RINEX file from a buffer
    def parseFromBuffer(self, buffer):

        # Create a string IO object based on the buffer
        bufferIO = io.StringIO(buffer)

        logging.debug("parseFromBuffer called")

        # Initialise the header variable
        header = []

        # Read the first line
        line = bufferIO.readline()
        
        while (line != None) :
            # Add the line to the header variable
            header.append(line)

            # Check whether this is the end of header line
            if (re.search(".*END OF HEADER", line) != None):
                logging.debug("End of header found")
                # Drop out of the while loop
                break

            # Get the next line
            line = bufferIO.readline()
        
        # Now parse the header
        self._parseHeader(header)

        # Now drop into a loop to parse the ephemerides while there is data
        # still in the buffer
        while(bufferIO.readable()):

            # Each ephemeris is 8 lines long, so drop into a loop to
            # read 8 lines (cannot use readlines as this only allows specification
            # or a number of bytes to read, which means we may get more that 8 lines
            # if a RINEX file that is missing some of the spare fields)
            ephemeris = []
            for i in range(0,8):
                line = bufferIO.readline()
                if line == '':
                    break
                else :
                    ephemeris.append(line)

            # Check that we were able to read 8 lines
            if len(ephemeris) != 8:
                break

            # Pass into function to parse the ephemeride
            parsedEphemeris = self._parseEphemeris(ephemeris)

            # Append the ephemeris to the content array
            self.contents['Ephemerides'].append(parsedEphemeris)

        return


    # Method to parse a RINEX from a file
    def parseFromFile(self, fileName):
        logging.debug("parseFromFile called")
        # Open the file that we have been passed
        with open(fileName) as inputFile :
            # Read the entire file
            buffer = inputFile.read()
            # Call the method to parse the contents
            parsedFile = self.parseFromBuffer(buffer)

        # Return the parsed data
        return parsedFile
    
    # Method to generate the header
    #  +--------------------+------------------------------------------+------------+
    #  |RINEX VERSION / TYPE| - Format version (2)                     |   I6,14X,  |
    #  |                    | - File type ('N' for Navigation data)    |   A1,19X   |
    #  +--------------------+------------------------------------------+------------+
    #  |PGM / RUN BY / DATE | - Name of program creating current file  |     A20,   |
    #  |                    | - Name of agency  creating current file  |     A20,   |
    #  |                    | - Date of file creation                  |     A20    |
    #  +--------------------+------------------------------------------+------------+
    # *|COMMENT             | Comment line(s)                          |     A60    |*
    #  +--------------------+------------------------------------------+------------+
    # *|ION ALPHA           | Ionosphere parameters A0-A3 of almanac   |  2X,4D12.4 |*
    #  |                    | (page 18 of subframe 4)                  |            |
    #  +--------------------+------------------------------------------+------------+
    # *|ION BETA            | Ionosphere parameters B0-B3 of almanac   |  2X,4D12.4 |*
    #  +--------------------+------------------------------------------+------------+
    # *|DELTA-UTC: A0,A1,T,W| Almanac parameters to compute time in UTC| 3X,2D19.12,|*
    #  |                    | (page 18 of subframe 4)                  |     2I9    |
    #  |                    | A0,A1: terms of polynomial               |            |
    #  |                    | T    : reference time for UTC data       |            |
    #  |                    | W    : UTC reference week number.        |            |
    #  |                    |        Continuous number, not mod(1024)! |            |
    #  +--------------------+------------------------------------------+------------+
    # *|LEAP SECONDS        | Delta time due to leap seconds           |     I6     |*
    #  +--------------------+------------------------------------------+------------+
    #  |END OF HEADER       | Last record in the header section.       |    60X     |
    #  +--------------------+------------------------------------------+------------+
    def _generateHeader(self):
        # Declare the buffer to store the header lines
        headerBuffer = []

        # First header line
        line = f"{self.contents['Header']['Version']: 6.2f}"
        line += " " * 14
        line += "NAVIGATION DATA".ljust(20, " ")
        line += " " * 20
        line += "RINEX VERSION / TYPE".ljust(20, " ")
        headerBuffer.append(line)

        # Second header line
        line = "rinexPython".ljust(20, " ")
        line += "Internal".ljust(20, " ")
        now = datetime.now()
        line += now.strftime("%d-%b-%y %H:%M").ljust(20, " ")
        line += "PGM / RUN BY / DATE".ljust(20, " ")
        headerBuffer.append(line)

        # Add the end of header line
        line = " " * 60 + "END OF HEADER".ljust(20, " ")
        headerBuffer.append(line)
        
        return headerBuffer
    
    # Generate the first ephemeris line
    # +--------------------+------------------------------------------+------------+
    # |PRN / EPOCH / SV CLK| - Satellite PRN number                   |     I2,    |
    # |                    | - Epoch: Toc - Time of Clock             |            |
    # |                    |          year         (2 digits)         |    5I3,    |
    # |                    |          month                           |            |
    # |                    |          day                             |            |
    # |                    |          hour                            |            |
    # |                    |          minute                          |            |
    # |                    |          second                          |    F5.1,   |
    # |                    | - SV clock bias       (seconds)          |  3D19.12   |
    # |                    | - SV clock drift      (sec/sec)          |            |
    # |                    | - SV clock drift rate (sec/sec2)         |            |
    # +--------------------+------------------------------------------+------------+
    def _generateEphemerisLine1(self, ephemeris):
        line = f"{ephemeris['PRN']:2d}"
        line += f"{ephemeris['TocYear']: 3d}"
        line += f"{ephemeris['TocMonth']: 3d}"
        line += f"{ephemeris['TocDay']: 3d}"
        line += f"{ephemeris['TocHr']: 3d}"
        line += f"{ephemeris['TocMin']: 3d}"
        line += f"{ephemeris['TocSec']: 5.1f}"
        line += f"{ephemeris['ClockBias']:19.12E}"
        line += f"{ephemeris['ClockDrift']:19.12E}"
        line += f"{ephemeris['ClockDriftRate']:19.12E}"
        return line

    # Generate the second ephemeris line
    # +--------------------+------------------------------------------+------------+
    # | BROADCAST ORBIT - 1| - IODE Issue of Data, Ephemeris          | 3X,4D19.12 |
    # |                    | - Crs                 (meters)           |            |
    # |                    | - Delta n             (radians/sec)      |            |
    # |                    | - M0                  (radians)          |            |
    # +--------------------+------------------------------------------+------------+
    def _generateEphemerisLine2(self, ephemeris):
        line = " " * 3
        line += f"{ephemeris['IODE']:19.12E}"
        line += f"{ephemeris['Crs']:19.12E}"
        line += f"{ephemeris['DeltaN']:19.12E}"
        line += f"{ephemeris['M0']:19.12E}"
        return line

    # Generate the third ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  | BROADCAST ORBIT - 2| - Cuc                 (radians)          | 3X,4D19.12 |
    #  |                    | - e Eccentricity                         |            |
    #  |                    | - Cus                 (radians)          |            |
    #  |                    | - sqrt(A)             (sqrt(m))          |            |
    #  +--------------------+------------------------------------------+------------+    
    def _generateEphemerisLine3(self, ephemeris):
        line = " " * 3
        line += f"{ephemeris['Cuc']:19.12E}"
        line += f"{ephemeris['e']:19.12E}"
        line += f"{ephemeris['Cus']:19.12E}"
        line += f"{ephemeris['SqrtA']:19.12E}"
        return line

    # Generate the fourth ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  | BROADCAST ORBIT - 3| - Toe Time of Ephemeris                  | 3X,4D19.12 |
    #  |                    |                       (sec of GPS week)  |            |
    #  |                    | - Cic                 (radians)          |            |
    #  |                    | - OMEGA               (radians)          |            |
    #  |                    | - CIS                 (radians)          |            |
    #  +--------------------+------------------------------------------+------------+
    def _generateEphemerisLine4(self, ephemeris):
        line = " " * 3
        line += f"{ephemeris['Toe']:19.12E}"
        line += f"{ephemeris['Cic']:19.12E}"
        line += f"{ephemeris['OMEGA_uc']:19.12E}"
        line += f"{ephemeris['Cis']:19.12E}"
        return line

    # Generate the fifth ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  | BROADCAST ORBIT - 4| - i0                  (radians)          | 3X,4D19.12 |
    #  |                    | - Crc                 (meters)           |            |
    #  |                    | - omega               (radians)          |            |
    #  |                    | - OMEGA DOT           (radians/sec)      |            |
    #  +--------------------+------------------------------------------+------------+
    def _generateEphemerisLine5(self, ephemeris):
        line = " " * 3
        line += f"{ephemeris['i0']:19.12E}"
        line += f"{ephemeris['Crc']:19.12E}"
        line += f"{ephemeris['omega_lc']:19.12E}"
        line += f"{ephemeris['OMEGA_DOT']:19.12E}"
        return line

    # Generate the sixth ephemeris line
    # +--------------------+------------------------------------------+------------+
    # | BROADCAST ORBIT - 5| - IDOT                (radians/sec)      | 3X,4D19.12 |
    # |                    | - Codes on L2 channel                    |            |
    # |                    | - GPS Week # (to go with TOE)            |            |
    # |                    |   Continuous number, not mod(1024)!      |            |
    # |                    | - L2 P data flag                         |            |
    # +--------------------+------------------------------------------+------------+
    def _generateEphemerisLine6(self, ephemeris):
        line = " " * 3
        line += f"{ephemeris['IDOT']:19.12E}"
        line += f"{ephemeris['L2_Codes']:19.12E}"
        line += f"{ephemeris['GpsWeek']:19.12E}"
        line += f"{ephemeris['L2_P']:19.12E}"
        return line

    # Generate the seventh ephemeris line
    # +--------------------+------------------------------------------+------------+
    # | BROADCAST ORBIT - 6| - SV accuracy         (meters)           | 3X,4D19.12 |
    # |                    | - SV health           (MSB only)         |            |
    # |                    | - TGD                 (seconds)          |            |
    # |                    | - IODC Issue of Data, Clock              |            |
    # +--------------------+------------------------------------------+------------+    
    def _generateEphemerisLine7(self, ephemeris):
        line = " " * 3
        line += f"{ephemeris['SV_Accuracy']:19.12E}"
        line += f"{ephemeris['SV_Health']:19.12E}"
        line += f"{ephemeris['TGD']:19.12E}"
        line += f"{ephemeris['IODC']:19.12E}"
        return line

    # Generate the eighth ephemeris line
    #  +--------------------+------------------------------------------+------------+
    #  | BROADCAST ORBIT - 7| - Transmission time of message           | 3X,4D19.12 |
    #  |                    |         (sec of GPS week, derived e.g.   |            |
    #  |                    |    from Z-count in Hand Over Word (HOW)  |            |
    #  |                    | - spare                                  |            |
    #  |                    | - spare                                  |            |
    #  |                    | - spare                                  |            |
    #  +--------------------+------------------------------------------+------------+ 
    def _generateEphemerisLine8(self, ephemeris):
        line = " " * 3
        line += f"{ephemeris['TxTime']:19.12E}"
        line += f"{0:19.12E}"
        line += f"{0:19.12E}"
        line += f"{0:19.12E}"
        return line
    
    def _generateEphemeris(self, ephemeris):
        # Declare the buffer to store the ephemeris in
        ephemerisBuffer = []

        # Generate all the lines
        ephemerisBuffer.append(self._generateEphemerisLine1(ephemeris))
        ephemerisBuffer.append(self._generateEphemerisLine2(ephemeris))
        ephemerisBuffer.append(self._generateEphemerisLine3(ephemeris))
        ephemerisBuffer.append(self._generateEphemerisLine4(ephemeris))
        ephemerisBuffer.append(self._generateEphemerisLine5(ephemeris))
        ephemerisBuffer.append(self._generateEphemerisLine6(ephemeris))
        ephemerisBuffer.append(self._generateEphemerisLine7(ephemeris))
        ephemerisBuffer.append(self._generateEphemerisLine8(ephemeris))

        return ephemerisBuffer
    

    # Define a method which will calculate a value that we can use to sort
    # ephemerides. 
    def _sortFunction(self, eph):
        # We want the following sort order
        # - Year
        # - Month
        # - Day
        # - Hour
        # - Min
        # - Sec
        # - PRN
        # Compute a value that will create this
        return eph['PRN'] + \
            eph['TocSec']   * 33  + \
            eph['TocMin']   * 33 * 60 + \
            eph['TocHr']    * 33 * 60 * 60 + \
            eph['TocDay']  * 33 * 60 * 60 * 24 + \
            eph['TocMonth'] * 33 * 60 * 60 * 24 * 31 + \
            eph['TocYear']  * 33 * 60 * 60 * 24 * 31 * 12 
            
            

    
    # Method to write a RINEX file to a buffer
    def writeToBuffer(self):

        # Delare the output buffer
        rinexBuffer = []

        # Generate the header and apply to the buffer
        rinexBuffer.extend(self._generateHeader())

        # Sort the ephemeris before we write
        self.contents['Ephemerides'].sort(key=self._sortFunction)

        # Loop over all the ephemerides and add them to the buffer
        for ephemeris in self.contents['Ephemerides']:
            rinexBuffer.extend(self._generateEphemeris(ephemeris))

        return rinexBuffer
    
    # Method to write a RINEX file directly to a file
    def writeToFile(self, filename):
        # Call the method to generate into a buffer
        rinexBuffer = self.writeToBuffer()

        # Write the buffer to a file
        with open(filename, 'w') as outputFile:
            # Write all the lines to the file
            for line in rinexBuffer:
                outputFile.write(line + "\n")

        return
    
    # Method to get the header
    def getHeader(self):
        return self.contents['Header']

    # Method to return the number of Ephemerides
    def getNumEphemerides(self):
        return len(self.contents['Ephemerides'])
    
    # Method to get the array of ephemerides
    def getEphemerides(self):
        return self.contents['Ephemerides']
    
    # Method to retuern a Python datetime object for an ephemeris ephoch
    def getEphemerisDateTime(self, ephemeris):
        # Generate the full year
        if (ephemeris['TocYear'] >= 80):
            year = ephemeris['TocYear'] + 1900
        else:
            year = ephemeris['TocYear'] + 2000
        return datetime( \
            year, \
            ephemeris['TocMonth'], \
            ephemeris['TocDay'], \
            ephemeris['TocHr'], \
            ephemeris['TocMin'], \
            int(ephemeris['TocSec']))

    def _caluculateGPSTime(self, inDate):
        # Define the GPS time epoch
        epoch = datetime(1980, 1, 6, 0, 0, 0)

        # Calculate the time since epoch
        timeSinceEpoch = inDate - epoch

        # Calculuate the gps week
        gpsWeek = int(timeSinceEpoch.days / 7)

        # Calculate the number of seconds since the start of the GPS week
        gpsWeekSeconds = (timeSinceEpoch - timedelta(gpsWeek*7)).total_seconds()

        return [gpsWeek, gpsWeekSeconds]

    def _calculateIodcIode(self, toe):
        # Derive a IODC / IODE from the toe.
        # NOTE : The spec does not define an approach to calculating 
        # IODE/IODC, instead it states that they are used for detecting
        # that we have matching subframes therefore should not repeat 
        # regularly.
        # The algorithm below ensures that a new value is used every 
        # 10 minutes, and that they do not repeat within 2 days
        return int((toe/600) % 144)


           
    # Define a method to update an ephemeris to a specific date/time
    def changeEphemerisTime(self, originalEphemeris, newDateTime):

        # Define the value for the earth's gravitatinal constant
        WGS384_EARTH_GRAV_CONSTANT = 3.986005e14
        WGS384_EARTH_ROTATIONAL_RATE = 7.2921151467e-5

        # Define the number of seconds in a week
        SECONDS_IN_WEEK = 60*60*24*7

        # Declare the updated ephemeris object, and inintialise with the
        # original ephemeris
        updatedEphemeris = copy.copy(originalEphemeris)
        
        # Update the epoch time
        # Year - in short form
        updatedEphemeris['TocYear']     = int(newDateTime.strftime("%y"))
        updatedEphemeris['TocMonth']    = newDateTime.month
        updatedEphemeris['TocDay']      = newDateTime.day
        updatedEphemeris['TocHr']       = newDateTime.hour
        updatedEphemeris['TocMin']      = newDateTime.minute
        updatedEphemeris['TocSec']      = newDateTime.second

        # Calculate and set the updated GPS time
        [gpsWeek, gpsWeekSeconds] = self._caluculateGPSTime(newDateTime)
        updatedEphemeris['GpsWeek']     = gpsWeek
        updatedEphemeris['Toe']         = gpsWeekSeconds

        # Update the IODC and IODE values
        updatedEphemeris['IODE']        = self._calculateIodcIode(gpsWeekSeconds)
        updatedEphemeris['IODC']        = self._calculateIodcIode(gpsWeekSeconds)


        # Update the orbital paramaters of the ephemeris
        # Calculate the working paramaters
        # These are taken from the GPS ICD (https://www.gps.gov/technical/icwg/IS-GPS-200M.pdf)
        # A = semi-major axis
        A = originalEphemeris['SqrtA'] * originalEphemeris['SqrtA']
        # n0 = computed mean motion 
        n0 = math.sqrt(WGS384_EARTH_GRAV_CONSTANT/pow(A,3))
        # n = corrected mean motion
        n = n0 + originalEphemeris['DeltaN']
        
        # Calculate the time difference, in seconds
        timeDiff = (newDateTime - self.getEphemerisDateTime(originalEphemeris)).total_seconds()

        # Calculate the updated mean anomaly
        mk = self._unwrapAngle(originalEphemeris['M0'] + (timeDiff * n))
        
        # Calculate the eccentric anomaly iteratively
        Ek = mk
        EkOld = mk + 1.0    # Set to +1 so that they are different to start the iterations
        # Drop into a look to detect when the process has converged
        while (abs(Ek - EkOld) < 1.0E-14):
            EkOld = Ek
            Ek = EkOld + ( \
                (mk-EkOld + originalEphemeris['e']*math.sin(EkOld)) /\
                (1.0 - originalEphemeris['e']*math.cos(EkOld)) \
                 
            )

        # True anomaly
        vk = 2.0 * math.atan( \
            math.sqrt((1+originalEphemeris['e'])/(1-originalEphemeris['e'])) * \
            math.tan(Ek/2)
        )    
            
        # Argument of latutide
        pk = vk + originalEphemeris['omega_lc']

        # Second harmonic pertubations        
        deltaik = originalEphemeris['Cis']*math.sin(2*pk) + originalEphemeris['Cic']*math.cos(2*pk)

        # Calculate the updated inclination
        ik = originalEphemeris['i0'] + deltaik + \
            (originalEphemeris['IDOT'] * timeDiff)

        # Update OMEGA 0
        # Work out the difference in GPS weeks
        weekDiff = gpsWeek - originalEphemeris['GpsWeek']

        # Calculate the updated OmegaZ for the start of the week
        O0 = self._unwrapAngle(originalEphemeris['OMEGA_uc'] + \
            (originalEphemeris['OMEGA_DOT'] - WGS384_EARTH_ROTATIONAL_RATE)*(weekDiff*SECONDS_IN_WEEK))

        # Update the values in the updated ephemeris
        updatedEphemeris['M0'] = mk
        updatedEphemeris['i0'] = ik
        updatedEphemeris['OMEGA_uc'] = O0

        # Return the update ephemeris
        return updatedEphemeris
  


        
        