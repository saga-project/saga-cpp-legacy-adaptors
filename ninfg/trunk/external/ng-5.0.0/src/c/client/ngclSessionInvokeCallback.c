/* This file is generate by ngclSessionInvokeCallback.sh */
typedef void (*ngcllCallbackFunc0)(void);
typedef void (*ngcllCallbackFunc1)(void *);
typedef void (*ngcllCallbackFunc2)(void *, void *);
typedef void (*ngcllCallbackFunc3)(void *, void *, void *);
typedef void (*ngcllCallbackFunc4)(void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc5)(void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc6)(void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc7)(void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc8)(void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc9)(void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc10)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc11)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc12)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc13)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc14)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc15)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc16)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc17)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc18)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc19)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc20)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc21)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc22)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc23)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc24)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc25)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc26)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc27)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc28)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc29)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc30)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc31)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
typedef void (*ngcllCallbackFunc32)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
/*
 * Invoke Callback
 */
int
ngcliSessionInvokeCallback(
    void (*callbackFunc)(void),
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcliSessionInvokeCallback";

    switch (arg->nga_nArguments){
    case 0:
        ((ngcllCallbackFunc0)(callbackFunc))();
        break;
    case 1:
        ((ngcllCallbackFunc1)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void);
        break;
    case 2:
        ((ngcllCallbackFunc2)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void);
        break;
    case 3:
        ((ngcllCallbackFunc3)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void);
        break;
    case 4:
        ((ngcllCallbackFunc4)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void);
        break;
    case 5:
        ((ngcllCallbackFunc5)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void);
        break;
    case 6:
        ((ngcllCallbackFunc6)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void);
        break;
    case 7:
        ((ngcllCallbackFunc7)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void);
        break;
    case 8:
        ((ngcllCallbackFunc8)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void);
        break;
    case 9:
        ((ngcllCallbackFunc9)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void);
        break;
    case 10:
        ((ngcllCallbackFunc10)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void);
        break;
    case 11:
        ((ngcllCallbackFunc11)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void);
        break;
    case 12:
        ((ngcllCallbackFunc12)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void);
        break;
    case 13:
        ((ngcllCallbackFunc13)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void);
        break;
    case 14:
        ((ngcllCallbackFunc14)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void);
        break;
    case 15:
        ((ngcllCallbackFunc15)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void);
        break;
    case 16:
        ((ngcllCallbackFunc16)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void);
        break;
    case 17:
        ((ngcllCallbackFunc17)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void);
        break;
    case 18:
        ((ngcllCallbackFunc18)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void);
        break;
    case 19:
        ((ngcllCallbackFunc19)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void);
        break;
    case 20:
        ((ngcllCallbackFunc20)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void);
        break;
    case 21:
        ((ngcllCallbackFunc21)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void);
        break;
    case 22:
        ((ngcllCallbackFunc22)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void,
           arg->nga_argument[21].ngae_pointer.ngap_void);
        break;
    case 23:
        ((ngcllCallbackFunc23)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void,
           arg->nga_argument[21].ngae_pointer.ngap_void,
           arg->nga_argument[22].ngae_pointer.ngap_void);
        break;
    case 24:
        ((ngcllCallbackFunc24)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void,
           arg->nga_argument[21].ngae_pointer.ngap_void,
           arg->nga_argument[22].ngae_pointer.ngap_void,
           arg->nga_argument[23].ngae_pointer.ngap_void);
        break;
    case 25:
        ((ngcllCallbackFunc25)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void,
           arg->nga_argument[21].ngae_pointer.ngap_void,
           arg->nga_argument[22].ngae_pointer.ngap_void,
           arg->nga_argument[23].ngae_pointer.ngap_void,
           arg->nga_argument[24].ngae_pointer.ngap_void);
        break;
    case 26:
        ((ngcllCallbackFunc26)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void,
           arg->nga_argument[21].ngae_pointer.ngap_void,
           arg->nga_argument[22].ngae_pointer.ngap_void,
           arg->nga_argument[23].ngae_pointer.ngap_void,
           arg->nga_argument[24].ngae_pointer.ngap_void,
           arg->nga_argument[25].ngae_pointer.ngap_void);
        break;
    case 27:
        ((ngcllCallbackFunc27)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void,
           arg->nga_argument[21].ngae_pointer.ngap_void,
           arg->nga_argument[22].ngae_pointer.ngap_void,
           arg->nga_argument[23].ngae_pointer.ngap_void,
           arg->nga_argument[24].ngae_pointer.ngap_void,
           arg->nga_argument[25].ngae_pointer.ngap_void,
           arg->nga_argument[26].ngae_pointer.ngap_void);
        break;
    case 28:
        ((ngcllCallbackFunc28)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void,
           arg->nga_argument[21].ngae_pointer.ngap_void,
           arg->nga_argument[22].ngae_pointer.ngap_void,
           arg->nga_argument[23].ngae_pointer.ngap_void,
           arg->nga_argument[24].ngae_pointer.ngap_void,
           arg->nga_argument[25].ngae_pointer.ngap_void,
           arg->nga_argument[26].ngae_pointer.ngap_void,
           arg->nga_argument[27].ngae_pointer.ngap_void);
        break;
    case 29:
        ((ngcllCallbackFunc29)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void,
           arg->nga_argument[21].ngae_pointer.ngap_void,
           arg->nga_argument[22].ngae_pointer.ngap_void,
           arg->nga_argument[23].ngae_pointer.ngap_void,
           arg->nga_argument[24].ngae_pointer.ngap_void,
           arg->nga_argument[25].ngae_pointer.ngap_void,
           arg->nga_argument[26].ngae_pointer.ngap_void,
           arg->nga_argument[27].ngae_pointer.ngap_void,
           arg->nga_argument[28].ngae_pointer.ngap_void);
        break;
    case 30:
        ((ngcllCallbackFunc30)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void,
           arg->nga_argument[21].ngae_pointer.ngap_void,
           arg->nga_argument[22].ngae_pointer.ngap_void,
           arg->nga_argument[23].ngae_pointer.ngap_void,
           arg->nga_argument[24].ngae_pointer.ngap_void,
           arg->nga_argument[25].ngae_pointer.ngap_void,
           arg->nga_argument[26].ngae_pointer.ngap_void,
           arg->nga_argument[27].ngae_pointer.ngap_void,
           arg->nga_argument[28].ngae_pointer.ngap_void,
           arg->nga_argument[29].ngae_pointer.ngap_void);
        break;
    case 31:
        ((ngcllCallbackFunc31)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void,
           arg->nga_argument[21].ngae_pointer.ngap_void,
           arg->nga_argument[22].ngae_pointer.ngap_void,
           arg->nga_argument[23].ngae_pointer.ngap_void,
           arg->nga_argument[24].ngae_pointer.ngap_void,
           arg->nga_argument[25].ngae_pointer.ngap_void,
           arg->nga_argument[26].ngae_pointer.ngap_void,
           arg->nga_argument[27].ngae_pointer.ngap_void,
           arg->nga_argument[28].ngae_pointer.ngap_void,
           arg->nga_argument[29].ngae_pointer.ngap_void,
           arg->nga_argument[30].ngae_pointer.ngap_void);
        break;
    case 32:
        ((ngcllCallbackFunc32)(callbackFunc))(
           arg->nga_argument[0].ngae_pointer.ngap_void,
           arg->nga_argument[1].ngae_pointer.ngap_void,
           arg->nga_argument[2].ngae_pointer.ngap_void,
           arg->nga_argument[3].ngae_pointer.ngap_void,
           arg->nga_argument[4].ngae_pointer.ngap_void,
           arg->nga_argument[5].ngae_pointer.ngap_void,
           arg->nga_argument[6].ngae_pointer.ngap_void,
           arg->nga_argument[7].ngae_pointer.ngap_void,
           arg->nga_argument[8].ngae_pointer.ngap_void,
           arg->nga_argument[9].ngae_pointer.ngap_void,
           arg->nga_argument[10].ngae_pointer.ngap_void,
           arg->nga_argument[11].ngae_pointer.ngap_void,
           arg->nga_argument[12].ngae_pointer.ngap_void,
           arg->nga_argument[13].ngae_pointer.ngap_void,
           arg->nga_argument[14].ngae_pointer.ngap_void,
           arg->nga_argument[15].ngae_pointer.ngap_void,
           arg->nga_argument[16].ngae_pointer.ngap_void,
           arg->nga_argument[17].ngae_pointer.ngap_void,
           arg->nga_argument[18].ngae_pointer.ngap_void,
           arg->nga_argument[19].ngae_pointer.ngap_void,
           arg->nga_argument[20].ngae_pointer.ngap_void,
           arg->nga_argument[21].ngae_pointer.ngap_void,
           arg->nga_argument[22].ngae_pointer.ngap_void,
           arg->nga_argument[23].ngae_pointer.ngap_void,
           arg->nga_argument[24].ngae_pointer.ngap_void,
           arg->nga_argument[25].ngae_pointer.ngap_void,
           arg->nga_argument[26].ngae_pointer.ngap_void,
           arg->nga_argument[27].ngae_pointer.ngap_void,
           arg->nga_argument[28].ngae_pointer.ngap_void,
           arg->nga_argument[29].ngae_pointer.ngap_void,
           arg->nga_argument[30].ngae_pointer.ngap_void,
           arg->nga_argument[31].ngae_pointer.ngap_void);
        break;
    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Too many argument for callback.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}
