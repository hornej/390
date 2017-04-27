#include <stdint.h>//import standard int
#include "queue.h"//import queueu
#include "filter.h"//import header file
#include <stdio.h>//import standard input output
#include <stdlib.h>//import standard library
#include <string.h>//include for string library
#include <stdbool.h> //include to do boolean
#define Z_QUEUE_SIZE 10 //the z queue holds values to calculate future z values
#define Y_QUEUE_SIZE 11 //the y queue holds 11 values
#define X_QUEUE_SIZE 81 //the size of the X queue
#define OUTPUT_QUEUE_SIZE 2000 //the size of our output queue
#define LAST_OUTPUT_QUEUE_ELEMENT 1999//the last element in the 2000 size queue
#define START 0 //where to start in loops
#define INIT_VAL 0.0 //what to initialize our queues to
#define FILTER_IIR_FILTER_COUNT 10 //the IIR filter count, also the same size as the Z queues
#define IIR_COEF_COUNT 11 //the number of coefficients in the iir filters
#define FIR_COEF_COUNT 81 //the length ofour FIR filter
#define PADDING 1 //padding for iterating through filter
#define NAME_SIZE 10 //the possible size of our name
static queue_t outputQueue[FILTER_IIR_FILTER_COUNT]; //declaring array of output queues
static queue_t zQueue[FILTER_IIR_FILTER_COUNT]; //declaring zqueue
static queue_t yQueue; //the yQueue
static queue_t xQueue; //the xQueue (before FIR filters)

static double currentPowerValue[FILTER_FREQUENCY_COUNT] = {0,0,0,0,0,0,0,0,0,0};//initialize current power values
static double oldPowerValue[FILTER_FREQUENCY_COUNT] = {0,0,0,0,0,0,0,0,0,0};//initialize old power values
static bool firstTime[FILTER_FREQUENCY_COUNT] = {true,true,true,true,true,true,true,true,true,true};//initialize the first time to true

double power = START;//power is initially zero
double value = START;//power value is initially zero
static double newestValue[FILTER_FREQUENCY_COUNT] = {0,0,0,0,0,0,0,0,0,0};//newest value in each of the filters

//sets the current power values (used for testing, would never call otherwise)
void filter_setCurrentPowerValues(double powerValues[]){
    for (uint32_t i=0; i<FILTER_FREQUENCY_COUNT; i++) {//go through and put this in current power values
        currentPowerValue[i] = powerValues[i];//set the current power values
    }
}

//FIR coefficients
const static double firCoefficients[X_QUEUE_SIZE] = {
        0.0000000000000000e+00,
        -3.4697553399459508e-06,
        -7.7850690560244372e-05,
        -3.0560167958554243e-04,
        -7.2403570533778876e-04,
        -1.2997929631630020e-03,
        -1.9198462076123759e-03,
        -2.4034242485096121e-03,
        -2.5345873828604682e-03,
        -2.1102272425782206e-03,
        -9.9408874864585295e-04,
        8.3499026966131929e-04,
        3.2523992840831017e-03,
        5.9837889885614353e-03,
        8.6321353465182490e-03,
        1.0733978907177606e-02,
        1.1836695573974654e-02,
        1.1583560991039379e-02,
        9.7906102209981138e-03,
        6.4995858716329867e-03,
        1.9947403655450058e-03,
        -3.2425636392738287e-03,
        -8.7133324448821371e-03,
        -1.3888455250484197e-02,
        -1.8208144592980213e-02,
        -2.1126481042781160e-02,
        -2.2157252711410058e-02,
        -2.0917304004121819e-02,
        -1.7163692456809411e-02,
        -1.0821355655980001e-02,
        -1.9986843124796816e-03,
        9.0106711883599071e-03,
        2.1740340290056486e-02,
        3.5577532837982505e-02,
        4.9801377897779241e-02,
        6.3630563893993616e-02,
        7.6276740391701084e-02,
        8.6999620865181274e-02,
        9.5159533552880052e-02,
        1.0026332322090840e-01,
        1.0199999999999999e-01,
        1.0026332322090840e-01,
        9.5159533552880052e-02,
        8.6999620865181274e-02,
        7.6276740391701084e-02,
        6.3630563893993616e-02,
        4.9801377897779241e-02,
        3.5577532837982505e-02,
        2.1740340290056486e-02,
        9.0106711883599071e-03,
        -1.9986843124796816e-03,
        -1.0821355655980001e-02,
        -1.7163692456809411e-02,
        -2.0917304004121819e-02,
        -2.2157252711410058e-02,
        -2.1126481042781160e-02,
        -1.8208144592980213e-02,
        -1.3888455250484197e-02,
        -8.7133324448821371e-03,
        -3.2425636392738287e-03,
        1.9947403655450058e-03,
        6.4995858716329867e-03,
        9.7906102209981138e-03,
        1.1583560991039380e-02,
        1.1836695573974650e-02,
        1.0733978907177606e-02,
        8.6321353465182524e-03,
        5.9837889885614353e-03,
        3.2523992840831021e-03,
        8.3499026966131864e-04,
        -9.9408874864585295e-04,
        -2.1102272425782228e-03,
        -2.5345873828604673e-03,
        -2.4034242485096139e-03,
        -1.9198462076123743e-03,
        -1.2997929631630020e-03,
        -7.2403570533778963e-04,
        -3.0560167958554183e-04,
        -7.7850690560244548e-05,
        -3.4697553399459195e-06,
        0.0000000000000000e+00};

const static double iirACoefficientConstants[Z_QUEUE_SIZE][Y_QUEUE_SIZE] = {
        {-5.9637727070163997e+00, 1.9125339333078244e+01, -4.0341474540744173e+01, 6.1537466875368821e+01, -7.0019717951472202e+01, 6.0298814235238908e+01, -3.8733792862566332e+01, 1.7993533279581083e+01, -5.4979061224867767e+00, 9.0332828533799836e-01},
        {-4.6377947119071452e+00, 1.3502215749461570e+01, -2.6155952405269751e+01, 3.8589668330738320e+01, -4.3038990303252589e+01, 3.7812927599537076e+01, -2.5113598088113736e+01, 1.2703182701888053e+01, -4.2755083391143343e+00, 9.0332828533799814e-01},
        {-3.0591317915750906e+00, 8.6417489609637368e+00, -1.4278790253808808e+01, 2.1302268283304240e+01, -2.2193853972079143e+01, 2.0873499791105353e+01, -1.3709764520609323e+01, 8.1303553577931247e+00, -2.8201643879900344e+00, 9.0332828533799470e-01},
        {-1.4071749185996769e+00, 5.6904141470697560e+00, -5.7374718273676368e+00, 1.1958028362868911e+01, -8.5435280598354737e+00, 1.1717345583835968e+01, -5.5088290876998691e+00, 5.3536787286077665e+00, -1.2972519209655604e+00, 9.0332828533799980e-01},
        {8.2010906117760274e-01, 5.1673756579268604e+00, 3.2580350909220908e+00, 1.0392903763919191e+01, 4.8101776408669057e+00, 1.0183724507092506e+01, 3.1282000712126736e+00, 4.8615933365571973e+00, 7.5604535083144853e-01, 9.0332828533799969e-01},
        {2.7080869856154481e+00, 7.8319071217995537e+00, 1.2201607990980708e+01, 1.8651500443681556e+01, 1.8758157568004464e+01, 1.8276088095998926e+01, 1.1715361303018827e+01, 7.3684394621253020e+00, 2.4965418284511718e+00, 9.0332828533799703e-01},
        {4.9479835250075910e+00, 1.4691607003177603e+01, 2.9082414772101068e+01, 4.3179839108869345e+01, 4.8440791644688900e+01, 4.2310703962394356e+01, 2.7923434247706446e+01, 1.3822186510471017e+01, 4.5614664160654375e+00, 9.0332828533799958e-01},
        {6.1701893352279864e+00, 2.0127225876810350e+01, 4.2974193398071741e+01, 6.5958045321253593e+01, 7.5230437667866795e+01, 6.4630411355740080e+01, 4.1261591079244290e+01, 1.8936128791950622e+01, 5.6881982915180593e+00, 9.0332828533800336e-01},
        {7.4092912870072425e+00, 2.6857944460290160e+01, 6.1578787811202332e+01, 9.8258255839887511e+01, 1.1359460153696327e+02, 9.6280452143026380e+01, 5.9124742025776612e+01, 2.5268527576524320e+01, 6.8305064480743436e+00, 9.0332828533800535e-01},
        {8.5743055776347692e+00, 3.4306584753117903e+01, 8.4035290411037110e+01, 1.3928510844056831e+02, 1.6305115418161648e+02, 1.3648147221895817e+02, 8.0686288623299944e+01, 3.2276361903872200e+01, 7.9045143816244963e+00, 9.0332828533800003e-01}
};

const static double iirBCoefficientConstants[Z_QUEUE_SIZE][Y_QUEUE_SIZE] = {
        {9.0928451882350956e-10, -0.0000000000000000e+00, -4.5464225941175478e-09, -0.0000000000000000e+00, 9.0928451882350956e-09, -0.0000000000000000e+00, -9.0928451882350956e-09, -0.0000000000000000e+00, 4.5464225941175478e-09, -0.0000000000000000e+00, -9.0928451882350956e-10},
        {9.0928639888111007e-10, 0.0000000000000000e+00, -4.5464319944055494e-09, 0.0000000000000000e+00, 9.0928639888110988e-09, 0.0000000000000000e+00, -9.0928639888110988e-09, 0.0000000000000000e+00, 4.5464319944055494e-09, 0.0000000000000000e+00, -9.0928639888111007e-10},
        {9.0928646492642129e-10, 0.0000000000000000e+00, -4.5464323246321064e-09, 0.0000000000000000e+00, 9.0928646492642127e-09, 0.0000000000000000e+00, -9.0928646492642127e-09, 0.0000000000000000e+00, 4.5464323246321064e-09, 0.0000000000000000e+00, -9.0928646492642129e-10},
        {9.0928706182467255e-10, 0.0000000000000000e+00, -4.5464353091233625e-09, 0.0000000000000000e+00, 9.0928706182467251e-09, 0.0000000000000000e+00, -9.0928706182467251e-09, 0.0000000000000000e+00, 4.5464353091233625e-09, 0.0000000000000000e+00, -9.0928706182467255e-10},
        {9.0928658835421734e-10, 0.0000000000000000e+00, -4.5464329417710870e-09, 0.0000000000000000e+00, 9.0928658835421740e-09, 0.0000000000000000e+00, -9.0928658835421740e-09, 0.0000000000000000e+00, 4.5464329417710870e-09, 0.0000000000000000e+00, -9.0928658835421734e-10},
        {9.0928659616426674e-10, -0.0000000000000000e+00, -4.5464329808213341e-09, -0.0000000000000000e+00, 9.0928659616426682e-09, -0.0000000000000000e+00, -9.0928659616426682e-09, -0.0000000000000000e+00, 4.5464329808213341e-09, -0.0000000000000000e+00, -9.0928659616426674e-10},
        {9.0928348598308410e-10, -0.0000000000000000e+00, -4.5464174299154200e-09, -0.0000000000000000e+00, 9.0928348598308400e-09, -0.0000000000000000e+00, -9.0928348598308400e-09, -0.0000000000000000e+00, 4.5464174299154200e-09, -0.0000000000000000e+00, -9.0928348598308410e-10},
        {9.0929582752648066e-10, 0.0000000000000000e+00, -4.5464791376324036e-09, 0.0000000000000000e+00, 9.0929582752648073e-09, 0.0000000000000000e+00, -9.0929582752648073e-09, 0.0000000000000000e+00, 4.5464791376324036e-09, 0.0000000000000000e+00, -9.0929582752648066e-10},
        {9.0926389052076022e-10, 0.0000000000000000e+00, -4.5463194526038007e-09, 0.0000000000000000e+00, 9.0926389052076014e-09, 0.0000000000000000e+00, -9.0926389052076014e-09, 0.0000000000000000e+00, 4.5463194526038007e-09, 0.0000000000000000e+00, -9.0926389052076022e-10},
        {9.0906203307668878e-10, 0.0000000000000000e+00, -4.5453101653834434e-09, 0.0000000000000000e+00, 9.0906203307668868e-09, 0.0000000000000000e+00, -9.0906203307668868e-09, 0.0000000000000000e+00, 4.5453101653834434e-09, 0.0000000000000000e+00, -9.0906203307668878e-10}
};




void initZQueues() {//initializes the z queues
    for (uint32_t i=START; i<FILTER_IIR_FILTER_COUNT; i++) {//go through all of the spots in the filter
        char name[NAME_SIZE] = ""; //name of the queue
        sprintf(name, "%d", i); //create the name of the queue
        queue_init(&(zQueue[i]), Z_QUEUE_SIZE, name); //initialize the queue for each iir filter
        for (uint32_t j=START; j<Z_QUEUE_SIZE; j++) //set the values of that queue to zero
            queue_overwritePush(&(zQueue[i]), INIT_VAL);//push zeros onto the queueue
    }
}

void initYQueue(){//initializes the Y queue
    queue_init(&yQueue, Y_QUEUE_SIZE, "YQueue"); //initialize the queue for each iir filter
    for (uint32_t j=START; j<Y_QUEUE_SIZE; j++) //set the values of that queue to zero
        queue_overwritePush(&yQueue, INIT_VAL);//push zeros onto the queueue
}

void initXQueue(){//initializes the x queue
    queue_init(&xQueue, X_QUEUE_SIZE, "XQueue"); //initialize the queue for each iir filter
    for (uint32_t j=START; j<X_QUEUE_SIZE; j++) //set the values of that queue to zero
        queue_overwritePush(&xQueue, INIT_VAL);//push zeros onto the queueue
}

void initOutputQueues() {//initializes the output queues
    for (uint32_t i=START; i<FILTER_IIR_FILTER_COUNT; i++) {//go through all of the spots in the filter
        char name[NAME_SIZE] = ""; //name of the queue
        sprintf(name, "%d", i); //create the name of the queue
        queue_init(&(outputQueue[i]), OUTPUT_QUEUE_SIZE, name); //initialize the queue for each iir filter
        for (uint32_t j=START; j < OUTPUT_QUEUE_SIZE; j++) //set the values of that queue to zero
            queue_overwritePush(&(outputQueue[i]), INIT_VAL);//push zeros onto the queueue
    }
}
// Must call this prior to using any filter functions.
void filter_init(){
    initZQueues();//initialize our z queues
    initYQueue(); //initialize our y queues
    initXQueue(); //initialize our y queues
    initOutputQueues(); //initialize output queues
}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x){
    queue_overwritePush(&xQueue, x);//push new value onto the queue
}

// Fills a queue with the given fillValue.
void filter_fillQueue(queue_t* q, double fillValue){
    for (uint32_t j=START; j<queue_size(q); j++) //set the values of that queue to fillValue
        queue_overwritePush(q, fillValue);//push new values onto the queueue
}

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter(){
    double y = INIT_VAL; //initialize since a new y value will be present
    for (uint32_t i=0; i<FIR_COEF_COUNT; i++) { //iterate through all points on the filter
        y += queue_readElementAt(&xQueue, FIR_COEF_COUNT-PADDING-i) * firCoefficients[i];  // iteratively adds the (FIR coefficients * input) products.
    }
    queue_overwritePush(&yQueue, y);//push new value onto the queue
    return y;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber){
    double bSum = START; //the bSum starts as zero
    double aSum = START; //the aSum starts as zero
    for (uint32_t i=START; i<IIR_COEF_COUNT; i++) { //iterate through all points on the filter
        bSum += queue_readElementAt(&yQueue, IIR_COEF_COUNT-PADDING-i) * iirBCoefficientConstants[filterNumber][i];  // iteratively adds the (IIR coefficients * input) products.
        if (i != START){//see if it i
            aSum += queue_readElementAt(&zQueue[filterNumber], IIR_COEF_COUNT-PADDING-i) * iirACoefficientConstants[filterNumber][i-PADDING];  // iteratively adds the (IIR coefficients * input) products.
        }
    }
    double valToPush = bSum-aSum; //sum the total
    queue_overwritePush(&(zQueue[filterNumber]), valToPush);//push new value onto the queue
    queue_overwritePush(&(outputQueue[filterNumber]), valToPush);//push new value onto the queue
    return (valToPush);
}

// Use this to compute the power for values contained in an outputQueue.
// If force == true, then recompute power by using all values in the outputQueue.
// This option is necessary so that you can correctly compute power values the first time.
// After that, you can incrementally compute power values by:
// 1. Keeping track of the power computed in a previous run, call this prev-power.
// 2. Keeping track of the oldest outputQueue value used in a previous run, call this oldest-value.
// 3. Get the newest value from the power queue, call this newest-value.
// 4. Compute new power as: prev-power - (oldest-value * oldest-value) + (newest-value * newest-value).
// Note that this function will probably need an array to keep track of these values for each
// of the 10 output queues.
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch, bool debugPrint){
    static double oldestValue[FILTER_FREQUENCY_COUNT] = {START,START,START,START,START,START,START,START,START,START}; //set oldestValue array
    power = START; //reset power variable
    if(forceComputeFromScratch){ //if we are computing from scratch
        firstTime[filterNumber] = true; //set the first time bool to true
    }
    if (filterNumber >= FILTER_FREQUENCY_COUNT){ //if the filterNumber is greater than 10
        printf("Error, you’re accessing a power value "
                "that is out of range. Filter No: %d\n\r", filterNumber); //print error message
        return START; //leave function
    }
    if(firstTime[filterNumber]){ //compute for the first time
        firstTime[filterNumber] = false; //reset bool
        for (uint32_t j=0; j<OUTPUT_QUEUE_SIZE; j++){ //parse through the array
            value = queue_readElementAt(&(outputQueue[filterNumber]),j); //save the output values
            oldestValue[filterNumber] = queue_readElementAt(&(outputQueue[filterNumber]),START); //save the oldest output value
            oldPowerValue[filterNumber] = oldestValue[filterNumber]*oldestValue[filterNumber]; //compute the older power value
            power = (value*value)+power; //compute the power value
            currentPowerValue[filterNumber] = power; //save the currentPowerValue
        }
        return currentPowerValue[filterNumber]; //return the currentPowerValue
    }
    newestValue[filterNumber] = queue_readElementAt(&(outputQueue[filterNumber]),LAST_OUTPUT_QUEUE_ELEMENT); //set the newestValue as the last value in array
    oldPowerValue[filterNumber] = currentPowerValue[filterNumber]; //set the oldPowerValue
    currentPowerValue[filterNumber] = oldPowerValue[filterNumber] -
            (oldestValue[filterNumber] * oldestValue[filterNumber]) +
            (newestValue[filterNumber] * newestValue[filterNumber]); //compute currentPowerValue by substractubg the old value and adding the new value
    oldestValue[filterNumber] = queue_readElementAt(&(outputQueue[filterNumber]),START); //reset the oldestValue
    return currentPowerValue[filterNumber]; //return the POWAAA!!!
}

// Returns the last-computed output power value for the IIR filter [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber){
    return currentPowerValue[filterNumber];//return current power values
}

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared array
// so that they can be accessed from outside the filter software by the detector.
// Remember that when you pass an array into a C function, changes to the array within
// that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[]){
    for (uint32_t i=0; i<FILTER_FREQUENCY_COUNT; i++) {//go through all of the power values
        powerValues[i] = currentPowerValue[i];//copy into the array
    }
}

// Using the previously-computed power values that are current stored in currentPowerValue[] array,
// Copy these values into the normalizedArray[] argument and then normalize them by dividing
// all of the values in normalizedArray by the maximum power value contained in currentPowerValue[].
void filter_getNormalizedPowerValues(double normalizedArray[], uint16_t* indexOfMaxValue){
    for (uint32_t i=0; i<FILTER_FREQUENCY_COUNT; i++) {
        normalizedArray[i] = (currentPowerValue[i])/(currentPowerValue[*indexOfMaxValue]);//normalize the power
    }
}



/*********************************************************************************************************
 ********************************** Verification-assisting functions. *************************************
 ********* Test functions access the internal data structures of the filter.c via these functions. ********
 *********************** These functions are not used by the main filter functions. ***********************
 **********************************************************************************************************/

// Returns the array of FIR coefficients.
const double* filter_getFirCoefficientArray(){
    return firCoefficients;//returns our fir coefficients
}

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount(){
    return X_QUEUE_SIZE; //return the size of our coefficients
}

// Returns the array of coefficients for a particular filter number.
const double* filter_getIirACoefficientArray(uint16_t filterNumber){
    return iirACoefficientConstants[filterNumber]; //return the iirAcoefficient constants for the given filter
}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount(){
    return Y_QUEUE_SIZE; //returns the iira coefficient count
}

// Returns the array of coefficients for a particular filter number.
const double* filter_getIirBCoefficientArray(uint16_t filterNumber){
    return iirBCoefficientConstants[filterNumber]; //return the iirBcoefficient constants for the given filter

}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount(){
    return Y_QUEUE_SIZE; //returns the iirb coefficient count
}

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize(){
    return queue_size(&yQueue); //returns the size of the y queue
}

// Returns the decimation value.
uint16_t filter_getDecimationValue(){
    return FILTER_FIR_DECIMATION_FACTOR; //returns the decimation facotr
}

// Returns the address of xQueue.
queue_t* filter_getXQueue(){
    return &xQueue; //returns the xqueue
}

// Returns the address of yQueue.
queue_t* filter_getYQueue(){
    return &yQueue; //returns the yqueue
}

// Returns the address of zQueue for a specific filter number.
queue_t* filter_getZQueue(uint16_t filterNumber){
    return &(zQueue[filterNumber]); //returns the zqueue
}

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t* filter_getIirOutputQueue(uint16_t filterNumber){
    return &(outputQueue[filterNumber]);//return the iir output queue
}








