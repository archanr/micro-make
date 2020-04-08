#ifndef Target
#define Target

//target structure declaration
struct targets;

//targetList is a pointer to target structure
typedef struct targets* targetList;

//rule structure declaration
struct rulesAndDep;

//rdList is a pointer to rule structure
typedef struct rulesAndDep* rdList;

typedef struct rulesAndDep rdNode;
typedef struct targets tNode;

typedef struct targList {
	//data field
	struct targets* targ_payload;
	//next pointer
	struct targetLists* next;
} tList;

//a function pointer to processline
typedef void (funcPointerToProcesline)(char *);

//this function creates a target node and adds dependencies if there are any
targetList createTargets(targetList* head, char* line);

//this function adds a new target to the end of the targetList
targetList addTargets(targetList* current, char* targetLine);

//the function adds rules to the rule and dependency structure
void addRules(char *ruleLine, targetList currentTarget);

//the function adds dependencies into the rule and dependency structure, very similar to addRules function
void addDependencies(char *dependencyLine, targetList currentTarget);

//this function will search through targetList names and find a match if an identical match is found
targetList targetMatch(targetList current, char *name);

//this function decides whether or not the rules for a dependency should execute or not
//based on if the file exists or not and the dependencies are newer than the target
bool shouldExecute(targetList targetMatchHead);

//with a target name this function will try to find matching target and
//execute the rules and dependencies for it recursively
void executeRule(funcPointerToProcesline executeWithProcessLine, targetList* headTarget, char* targetname);


#endif