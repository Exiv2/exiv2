/***********************************************************************
 *
 * Description of file:
 *   Definition of FFF fileformat
 *
 ***********************************************************************/
#ifndef FFF_H
#define FFF_H

/*=====================================================================*/

#define FLIR_FILE_FORMAT           0x46464600               // FFF\0
#define FILE_FORMAT_VERSION        100                      // 1.00
#define FFF_EXT                  "FFF"                      // Default file name extension

#define SYSIMG_NAMELEN 15


enum {FFF_HDR_SWAP_PATTERN = 0x0001};

// Bit mask for trig types
#define FPGA_TRIG_TYPE_MARK			0x01
#define FPGA_TRIG_TYPE_MARK_START	0x02
#define FPGA_TRIG_TYPE_MARK_STOP	0x04
#define FPGA_TRIG_TYPE_MARK_ENABLE	0x08	// Enable image flow
#define FPGA_TRIG_TYPE_MARK_DISABLE	0x10	// Disable image flow

#define FPGA_TRIG_STATE_DEASSERTED	0
#define FPGA_TRIG_STATE_ASSERTED	1

struct _fpgaheader_t
{
	BYTE		        major;
	BYTE		        minor;
	BYTE		        dp1_trig_type;		// Digital port 1, trig type
	BYTE		        dp2_trig_type;		// Digital port 2, trig type
	BYTE		        dp1_frame_ts;		// Digital port 1, frame time stamp
	BYTE		        dp1_line_ts_MSB;	// Digital port 1, line time stamp (MSB)
	BYTE		        dp1_line_ts_LSB;	// Digital port 1, line time stamp (LSB)
	BYTE		        dp2_frame_ts;		// Digital port 2, frame time stamp
	BYTE		        dp2_line_ts_MSB;		// Digital port 2, line time stamp (MSB)
	BYTE		        dp2_line_ts_LSB;		// Digital port 2, line time stamp (LSB)

	BYTE		        dp1_trig_state;		// Digital port 1, trig state
	BYTE		        dp2_trig_state;		// Digital port 2, trig state

	BYTE		        frame_cnt_MSB;
	BYTE		        frame_cnt_LSB;

	BYTE		        reserved[18];
};

typedef	struct _fpgaheader_t	    FPGA_HEADER;

// main types

typedef enum {
    /* General tags */
    FFF_TAGID_FREE = 0,                 /* Marks unused tag descriptor */

    FFF_TAGID_Pixels = 1,
    FFF_TAGID_GainMap = 2,
    FFF_TAGID_OffsMap = 3,
    FFF_TAGID_DeadMap = 4,
    FFF_TAGID_GainDeadMap = 5,
    FFF_TAGID_CoarseMap = 6,
    FFF_TAGID_ImageMap = 7,

    FFF_TAGID_SubFlirFileHead = 0x1e,
    FFF_general_high = 0x1f,            /* Reserve space for other general
                                           tags */

    /* FLIR TAGs */
    FFF_TAGID_BasicData = 0x20,
    FFF_TAGID_Measure,
    FFF_TAGID_ColorPal,
    FFF_TAGID_TextComment,
    FFF_TAGID_VoiceComment,

    FFF_TAGID_matrix_high = 0x3f,       /* reserve space for other system
                                           image blocks */

    /* FLIR Boston reserved TAG number series */
    FFF_TAGID_Boston_reserved = 0x40,
    FFF_TAGID_Boston_reserved_high = 0x5f,

    FFF_highnum = 0x100                 /* Guarantee 2 bytes enum */
} TAG_MAIN_T;

/* Sub Tags for FFF_TAGID_Pixels */

enum {FFF_Pixels_BE = 1,                /* Big endian pixel data block */
      FFF_Pixels_LE = 2,                /* Little endian pixel data block */
      FFF_Pixels_PNG = 3                /* PNG compressed pixel data block (BE or LE) */
};
/* When appropriate, add subID enums for other TAG_MAIN_T too */


/* When subtype isn't used, fill subtype with FFF_SubID_default */
enum {FFF_SubID_default = 1};

#pragma pack(push,1)

typedef struct tagFLIRFILEHEAD
{
    char  szFormatID[4];         /* Fileformat ID 'FFF\0'  4   4       */
    char  szOrigin[16];          /* File origin           16  20       */
    unsigned long dwVersion;     /* File format version    4  24       */
    unsigned long dwIndexOff;    /* Pointer to indexes     4  28       */
    unsigned long dwNumUsedIndex;/* Number of indexes      4  32       */
    unsigned long dwNextID;      /* Next free index ID     4  36       */
    unsigned short usSwapPattern;/* Swap pattern (0=MSBF)  2  38       */
    unsigned short usSpare[7];   /* Spare                 14  52       */
    unsigned long reserved[2];   /* reserved               8  60       */
    unsigned long dwChecksum;    /* Head & index checksum  4  64 bytes */
} FLIRFILEHEAD;

typedef struct _geometric_info_t
{
                                  /** Size of one pixel in bytes.
                                      Normal size is 2 bytes (16 bit pixels)
                                      or 3 (for colorized YCbCr pixels) */
    unsigned short pixelSize;

    unsigned short imageWidth;    //!< Image width in pixels
    unsigned short imageHeight;   //!< Image height in pixels

                                  /** @name Upper left coordinates
                                      X and Y coordinates for upper left corner
                                      relative original in case this image is a
                                      cutout, normally 0 */
                                  /*@{*/
    unsigned short upperLeftX;
    unsigned short upperLeftY;
                                  /*@}*/

                                  /** @name Valid pixels
                                      The following four number identifies the
				      valid pixels area within the image.
				      Sometimes the first row and column only
				      contains calibration pixels that should not
				      be considered as real pixels */
                                  /*@{*/
    unsigned short firstValidX;   //!< Normally 0
    unsigned short lastValidX;    //!< Normally imageWidth - 1
    unsigned short firstValidY;   //!< Normally 0
    unsigned short lastValidY;    //!< Normally imageHeight - 1
                                  /*@}*/
    unsigned short detectorDeep;  //!< Number of bits from detector A/D

                                  /** Type of detector to be able to differ
                                      between technologies if necessary.
                                      Defined in AppCore/core_imgflow/imgflow_state.hpp */
    unsigned short detectorID;
                                  /**  Type of upsampling from Detector to IR pixels.
                                       Defined in AppCore/core_imgflow/imgflow_state.hpp */
    unsigned short upSampling;
    unsigned short frameCtr;      //!< Image number from frame buffer
    unsigned short minMeasRadius; //!< See AppCore/core_imgflow/imgflow_state.hpp for reference
    unsigned char  stripeFields;  //!< Number of striped fields this image consists of
    unsigned char  reserved;      //!< For future use - should be set to 0
    unsigned short reserved1;     //!< For future use - should be set to 0
} GEOMETRIC_INFO_T;               //!< sizeof struct == 32 bytes

struct IMAGEINFO_T
{
    unsigned long imageTime;      //!< Time in seconds since 1970-01-01 00:00 (UTC)
    unsigned long imageMilliTime; //!< Milliseconds since last second

    short timeZoneBias;           //!< Time zone bias in minutes
                                  //!    UTC = local time + bias
    short swReserved1;            //!< filler == 0
    long focusPos;                //!< Focus position as counter value
    float fTSTemp[7];             //!< Temp sensor values converted to Kelvin
    float fTSTempExt[4];          //!< Lens temp sensors et.c. Converted to Kelvin
    unsigned short trigInfoType;  //!< 0 = No info, 1 = THV 900 type info
    unsigned short trigHit;       //!< hit count - microsecs from trig
                                  //!    reference
    unsigned short trigFlags;     //!< trig flags, type dependent
    unsigned short reserved1;
    unsigned long  trigCount;     //!< trig counter
    short manipulType;            //!< defines how to interpret manipFactors
    short manipFactors[5];        //!< Used average factors
    /** Detecor settings - camera type dependent */
    long detectorPars[20];        //!< Currently used detector parameters like
                                  //!    used bias, offsets. Usage is camera
                                  //!    dependent
    long reserved[5];             //!< For future use

};                                //!< sizeof struct == 184 bytes

/**
   Distribution information
*/
struct DISTR_DATA_T
{
    /** Framegrab independent distribution data */
    char imgName[16];  /* (4 longs) */

    unsigned short distrLive;         //!< TRUE (1) when image distribution is
				      //!	'LIVE'. FALSE (0) otherwise
    unsigned short distrRecalled;     //!< TRUE (1) when image distribution is
                                      //!       recalled. FALSE (0) otherwise.
                                      //!< TRUE also implies that distrLive ==
                                      //!       FALSE
    long curGlobalOffset;
    float curGlobalGain;              //!< globalOffset/Gain to generate LUT from
                                      //!  updated continously when live only
#define regulationOn 1
    unsigned short regulMethodMask;   //!< Method used for o/g calculation
    unsigned short visualImage;       //!< TRUE (1) for TV (visual)
                                      //!  FALSE (0) for IR image
    float focusDistance;              //!< focusDistance in meters.
				      //!    0 means not defined.
				      //!    NOT calculated by image source

    unsigned short StripeHeight;      //!< 0 = not striped
    unsigned short StripeStart;       //!< Striping start line if striped
    unsigned short imageFreq;         //!< Image frequency, defines the nominal
                                      //!    image frequency in Hz
    unsigned short typePixStreamCtrlData;
                                      //!< 0 = no such data,
                                      //!    other types TBD
    unsigned short PixStreamDataLine;
                                      //!< At which line to find
                                      //!    PixStreamCtrlData if any

#define IMGSMSK_INVALID  0x1          // Bit set means that image is non-measurable. Possibly because of a ongoing NUC.
#define IMGSMSK_ILL_LENS  0x2         // Measurement functions etc should be invalidated.
#define IMGSMSK_NONLINEAR 0x4         // Bit set means that (live) image is nonlinear (and therefore non-measurable).
#define IMGSMSK_LSLOCKED  0x8         // Bit set means that (live) image should be locked for level/span/distr updates, but not palette changes
#define IMGSMSK_BMP_RECALL 0x10       // Bit set means that a CSI bmp image is recalled. (Lut shall be updated even if image is non-measurable)

    short errStatus;                  //!< bit field, mask definitions above

    unsigned short imageMilliFreq;    //!< Image frequency, milliHz part of imageFreq

    short reserved;                   //!< For future use
    long reserved2[3];
};                                    //!< sizeof struct == 64 bytes

typedef struct _pres_par_t {
    signed long level;           /* Level as normalized pixel value (apix), Level is defined as middle of
				                    span (in pixel units) */
    signed long span;            /* Span as normalized pixel value (apix) */
	BYTE reserved[40];
} PRES_PAR_T;                    /* sizeof struct == 48 bytes */

/**
 *  FFF tag index
 */
typedef struct tagFLIRFILEINDEX
{
   USHORT  wMainType;    /**< Main type of index     2   2
                              Should be one of TAG_MAIN_T        */
   USHORT wSubType;      /**< Sub type of index      2   4       */
   ULONG  dwVersion;     /**< Version for data       4   8       */
   ULONG  dwIndexID;     /**< Index ID               4  12       */
   ULONG  dwDataPtr;     /**< Pointer to data        4  16       */
   ULONG  dwDataSize;    /**< Size of data           4  20       */
   ULONG  dwParent;      /**< Parentnr               4  24
                              may be set to 0 when not used      */
   ULONG  dwObjectNr;    /**< This object nr         4  28
                              may be set to 0 when not used      */
   ULONG  dwChecksum;    /**< Data checksum          4  32 bytes

        dwChecksum is a checksum of the tag data block. It may be
        set to 0 when not used. The algorithm is CRC32 with the
        Ethernet polynom. */
} FLIRFILEINDEX;

/**
 *  Object parameters
 *
 */
struct OBJECT_PAR_T
{
    float emissivity;            //!< 0 - 1
    float objectDistance;        //!< Meters
    float ambTemp;               //!< degrees Kelvin
    float atmTemp;               /**< degrees Kelvin
                                      - should be set == ambTemp for basic S/W */

    float extOptTemp;            /**< degrees Kelvin
                                      - should be set = ambTemp for basic S/W */
    float extOptTransm;          //!< 0 - 1: should be set = 1.0 for basic S/W
    float estAtmTransm;          //!< 0 - 1: should be set = 0.0 for basic S/W

    float relHum;                //!< relative humidity
    long reserved[4];            //!< For future use - should be set to 0
};                               //!< sizeof struct == 48 bytes

struct TEMP_CALIB_T
{
    long  Reserved1[2];
    float R;                      //!< Calibration constant R
    float B;                      //!< Calibration constant B
    float F;                      //!< Calibration constant F

    long  Reserved2[3];

    float alpha1;                 /* Attenuation for atmosphere without water vapor. */
    float alpha2;                 /* Attenuation for atmosphere without water vapor */
    float beta1;                  /* Attenuation for water vapor */
    float beta2;                  /* Attenuation for water vapor */
    float X;                      /* Scaling factor for attenuation. */

    long  Reserved3[3];

    float tmax;                   //!< Upper temp limit [K] when calibrated for
			                   	  //!<   current temp range
    float tmin;                   //!< Lower temp limit [K] when calibrated for
				                  //!<   current temp range
    float tmaxClip;               //!< Upper temp limit [K] over which the
                                  //!<   calibration becomes invalid
    float tminClip;               //!< Lower temp limit [K] under which the
                                  //!<   calibration becomes invalid
    float tmaxWarn;               //!< Upper temp limit [K] over which the
                                  //!<   calibration soon will become invalid
    float tminWarn;               //!< Lower temp limit [K] under which the
                                  //!<   calibration soon will become invalid
    float tmaxSaturated;          //!< Upper temp limit [K] over which pixels
                                  //!<   should be presented with overflow color
    float tminSaturated;          //!< Lower temp limit [K] for saturation
                                  //!<   (see also ADJUST_PAR_T:ipixOverflow).
                                  //!<   ipix over/under flow should be calculated
                                  //!<   by imgsrc from tmin/maxSaturated.
                                  //!<   LUT handler should look at ipix
                                  //!<   over/underflow.
    long Reserved4[9];
};                                //!< sizeof struct == 132 bytes

/*
  Adjust parameters
  -----------------
*/

typedef struct _adjust_par_t {
    long  normOffset;                 /* Temperature compensation offset
                                         (globalOffset) */
    float normGain;                   /* Temperature compensation gain
                                         (globalGain) */
    unsigned short ipixUnderflow; /* Image pixel underflow limit */
    unsigned short ipixOverflow;  /* Image pixel overflow limit */
    long Reserved2[9];
} ADJUST_PAR_T;                   /* sizeof struct == 48 bytes */

/** Temp sensor data struct */
struct TEMPSENSOR_DATA_T
{
    float         fTSTemp;          //!< Converted to Kelvin
    char          pzTSName[SYSIMG_NAMELEN+1];
    ULONG         captureTime;      //!< TS updated; time in seconds since 1970-01-01 00:00
    ULONG         captureMilliTime; //!< TS updated; Milliseconds since last second
};                                  //!< sizeof struct == 28 bytes

/** Detector parameter struct */
struct DETECTOR_PARAM_T
{
    float fData;
    char  pzDPName[SYSIMG_NAMELEN+1];
};                                  //!< sizeof struct == 20 bytes


/**
    Extended image info (more tempsensor and detector data)
 */
struct EXTENDED_IMAGEINFO_T
{
    TEMPSENSOR_DATA_T tsData[20];       //!< Temp sensor data
    DETECTOR_PARAM_T  detectorPars[20]; //!< Currently used detector parameters like
                                        //!     used bias, offsets. Usage is camera
                                        //!     dependent
};                                      //!< sizeof struct == 960 bytes

struct _bidata_t
{
	GEOMETRIC_INFO_T    GeometricInfo; // 32 bytes
    OBJECT_PAR_T        ObjectParameters;
    TEMP_CALIB_T        CalibParameters;
    BYTE		        CalibInfo[564];
    ADJUST_PAR_T        AdjustParameters;
    PRES_PAR_T	        PresentParameters;
    BYTE				DisplayParameters[28];
    IMAGEINFO_T         ImageInfo;
    DISTR_DATA_T        DistributionData;
    EXTENDED_IMAGEINFO_T ExtendedImageInfo;
};

typedef	struct _bidata_t	    BI_DATA_T;

struct _fff_header_t
{
    FLIRFILEHEAD    header;
    FLIRFILEINDEX   itag;
    FLIRFILEINDEX   btag;
};

typedef struct _fff_header_t        FFF_FILE_HEADER;

#define BASICDATAREV 102
#define IMAGEPIXELREV   100L

#pragma pack(pop)

/*---------------------------------------------------------------------*/
/* #endif for fff.h include                                            */
/*---------------------------------------------------------------------*/
#endif
