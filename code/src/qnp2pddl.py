from typing import Dict
class QnpParser:
    def __init__(self,debug=False) -> None:
        # file_content = io.StringIO(file_str)
        self.debug = debug
        ###############################
        
        pass

    def _readOneLine(self):
        return self.qnp_content.readline().strip()
        ...

    def _readNumNameValS(self):
        res = []
        splits = self._readOneLine().split()
        num = int(splits[0])
        for i in range(num):
            start = 2*(i+1) - 1
            res.append((splits[start],int(splits[start+1])))
        return res
    
    def loadQnp(self,file_content):
        
        self.qnp_content = file_content
        ### Domain ###
        self._loadProblemName()
        self._loadFeatures()
        self._loadInit()
        self._loadGoal()
        self._loadActions()
        ...
    
    def _loadProblemName(self):
        self.problem_name = self._readOneLine()
        self.problem_name = self.problem_name.replace(" ","-") # ADD FOR QNP-ABSTRACTION
        if self.debug:
            print(self.problem_name)
        ...

    def _loadFeatures(self):
        self.features = {}
        self.numeric_features = []
        for name_type in self._readNumNameValS():
            self.features[name_type[0]] = name_type[1]
            if name_type[1] == 1:
                self.numeric_features.append(name_type[0])
        if self.debug:
            print(self.features)
        ...


    ### Problem ###
    def _loadInit(self):
        self.inital = []
        self.inital = self._readNumNameValS()
        # for name_val in self._readNumNameValS():
        #     self.inital.append([name_val[0],name_val[1]])
        # splits = self._readOneLine().split()
        # num = int(splits[0])
        # for i in range(num):
        #     start = 2*(i+1) - 1
        #     v_name,v_val = (splits[start],splits[start+1])
        #     self.inital.append((v_name,int(v_val)))
        ...
            
        if self.debug:
            print(self.inital)

    def _loadGoal(self):
        self.goal = []
        self.goal = self._readNumNameValS()
        # for name_val in self._readNumNameValS():
        #     self.goal.append([name_val[0],name_val[1]])
        # splits = self._readOneLine().split()
        # num = int(splits[0])
        # for i in range(num):
        #     start = 2*(i+1) - 1
        #     v_name,v_val = (splits[start],splits[start+1])
        #     self.goal.append((v_name,int(v_val)))
        ...
            
        if self.debug:
            print(self.goal)
        ...


    def _loadActions(self):
        self.actions : dict[str,QnpAction] = {} 
        splits = self._readOneLine().split()
        num = int(splits[0])
        for _ in range(num):
            action_name = self._readOneLine().lower()
            action_name = action_name.replace(", ","-").replace(",","-").replace("((","-").replace("(","-").replace(")","") # ADD FOR QNP-ABSTRACTION
            action = QnpAction(action_name)
            for name,pre in self._readNumNameValS():
                action.addPrecondition(name,pre)
            for name,eff in self._readNumNameValS():
                action.addEffect(name,eff)
                if name in self.numeric_features:
                    # numerical
                    if eff == 1:
                        action.incrementX(name)
                    elif eff == 0:
                        action.decrementX(name)
            self.actions[action_name] = action

        if self.debug:
            print(self.actions)     
        ...

class QnpAction:
    def __init__(self,name) -> None:
        self.action_name = name
        self.action_preconditions = []
        self.action_effects = []
        self.increment_names = set([])
        self.decrement_names = set([])
        pass

    def addPrecondition(self,v_name,v_pre):
        self.action_preconditions.append((v_name,v_pre))
        ...

    def addEffect(self,v_name,v_eff):
        self.action_effects.append((v_name,v_eff))
        ...

    def incrementX(self,v_name):
        self.increment_names.add(v_name)

    def decrementX(self,v_name):
        self.decrement_names.add(v_name)


class QnpPddl:
    def __init__(self,qnp : QnpParser,debug=False) -> None:
        self.qnp = qnp
        # self.domain_pddl = domain_pddl
        # self.problem_pddl = problem_pddl        
        self.debug = debug
        
    def _indentPrint(self,string,indent=0):
        return "    "*indent+string

    def toPddl(self,domain_pddl,problem_pddl):
        self.toDomainPddl(domain_pddl)
        self.toProblemPddl(problem_pddl)

    def _outDomainName(self,type:str):
        if type == "domain":
            res = f"(domain {self.qnp.problem_name})"
        if type == "problem":
            res = f"(problem {self.qnp.problem_name}) (:domain {self.qnp.problem_name})"

        if self.debug:
            print(res)        
        return res

    def toDomainPddl(self,domain_pddl):
        pddl_content = self.outDomainPddl()
        with open(domain_pddl,"w") as f:
            f.write(pddl_content)
        ...

    def outDomainPddl(self):
        pddl_content = ""
        pddl_content += "(define "
        pddl_content += self._outDomainName(type = "domain")
        pddl_content += "\n\n"
        pddl_content += self._outPredicates()
        pddl_content += "\n\n"
        pddl_content += self._outActions()
        pddl_content += ")"
        return pddl_content

    def toProblemPddl(self,problem_pddl):
        pddl_content = self.outProblemPddl()
        with open(problem_pddl,'w') as f:
            f.write(pddl_content)

    def outProblemPddl(self):
        pddl_content = ""
        pddl_content += "(define "
        pddl_content += self._outDomainName(type = "problem")
        pddl_content += "\n\n"
        pddl_content += self._outInit()
        pddl_content += "\n\n"
        pddl_content += self._outGoal()
        pddl_content += ")"
        return pddl_content
    
    def outFairProblemPddl(self):
        pddl_content = ""
        pddl_content += "(define "
        pddl_content += self._outDomainName(type = "problem")
        pddl_content += "\n\n"
        pddl_content += self._outInit()
        pddl_content += "\n\n"
        pddl_content += self._outGoal()
        pddl_content += "\n\n"
        pddl_content += self._outFair()
        pddl_content += ")"
        return pddl_content

    def getFeatureLiteral(self,name,positive=True):
        type = self.qnp.features[name]
        literal = f"({name}-gr0)" if type == 1 else f"({name})"
        if not positive:
            literal = f"(not {literal})"
        return literal

    def _outPredicates(self):
        res = self._indentPrint("(:predicates",indent=1)
        # for name,type in self.qnp.features.items():
        for name in self.qnp.features.keys():
            res += '\n'
            res += self._indentPrint(self.getFeatureLiteral(name),indent=2)
            # if type == 1:
            #     res += self._indentPrint(f"({name}-gr0)",indent=2)
            # elif type == 0:
            #     res += self._indentPrint(f"({name})",indent=2)
        res += ")"

        if self.debug:
            print(res)

        return res     
        ...
    
    def _outActions(self):
        res = ""
        for action in self.qnp.actions.values():
            res += self._indentPrint(f"(:action {action.action_name}\n",indent=1)
            res += self._indentPrint(":parameters ()\n",indent=2)

            res += self._indentPrint(f":precondition ",indent=2)
            pres = self._outActionPreconditions(action)
            if len(pres) > 1:
                res += "(and "
                res += " ".join(pres)
                res += ")"
                ...
            elif len(pres) == 1:
                res += pres[0]
            elif len(pres) == 0:
                res += "(and)"
            res += "\n"

            res += self._indentPrint(f":effect ",indent=2)
            # {action.action_effects}
            effs = self._outActionEffects(action)
            if len(effs) > 1:
                res += "(and "
                res += " ".join(effs)
                res += ")"
                ...
            elif len(effs) == 1:
                res += effs[0]
            elif len(effs) == 0:
                res += "()"
            ...

            res += ")\n\n"
        
        if self.debug:
            print(res)
        return res
    
    def _outActionPreconditions(self,action):
        '''return [ "(not (holding))", "(holding)", "(n-gr0)", "(not (n-gr0))"]'''
        res = []
        for name,val in action.action_preconditions:
            res.append(self.getFeatureLiteral(name,positive=val))#(f"({name}-gr0)")
            # if self.qnp.features[name] == 1:
            #     # numeric
            #     if val == 1:
            #         res.append(f"({name}-gr0)")
            #     elif val == 0:
            #         res.append(f"(not ({name}-gr0))")
            # elif self.qnp.features[name] == 0:
            #     res.append(f"({name})" if val == 1 else f"(not ({name}))")
        return res
    
    def _outActionEffects(self,action):
        '''return [ "(not (holding))", "(holding)", "(n-gr0)", "(oneof (n-gr0)(not (n-gr0)))"]'''
        res = []
        for name,val in action.action_effects:
            if name in self.qnp.numeric_features:
            # if self.qnp.features[name] == 1:
                if val == 1:
                    # inc
                    res.append(self.getFeatureLiteral(name))
                elif val == 0:
                    res.append(f"(oneof {self.getFeatureLiteral(name)}{self.getFeatureLiteral(name,positive=False)})")
            else:
            # elif self.qnp.features[name] == 0:
                res.append(self.getFeatureLiteral(name,positive=val))
        return res
    
    def _outInit(self):
        res = ""
        res += self._indentPrint(f"(:init ",indent=1)
        one_time = True
        for name,val in self.qnp.inital:          
            if val == 1:
                if one_time:
                    one_time = False
                    res += self.getFeatureLiteral(name)
                    continue      
                res += '\n'
                res += self._indentPrint(self.getFeatureLiteral(name),indent=2)
        res += ')'

        if self.debug:
            print(res)
        return res

    def _outGoal(self):
        res = ""
        res += self._indentPrint(f"(:goal ",indent=1)
        goals = [self.getFeatureLiteral(name,positive=val) for name,val in self.qnp.goal]
        if len(goals) > 1:
            res += "(and "
            res += " ".join(goals)
            res += ")"
            ...
        elif len(goals) == 1:
            res += goals[0]
        res += ")\n"
        
        if self.debug:
            print(res)
        return res

    def _outFair(self):
        res = ""
        fairness_dict = {key:{"d":[],"i":[]} for key in self.qnp.numeric_features} # {"n_y":{"a":[]},}
        for action in self.qnp.actions.values():
            for nf in action.decrement_names:
                fairness_dict[nf]["d"].append(action.action_name)
            for nf in action.increment_names:
                fairness_dict[nf]["i"].append(action.action_name)
        for n_dict in fairness_dict.values():
            if n_dict["d"]:
                res += self._indentPrint("(:fairness\n",indent=1)
                res += self._indentPrint(":a",indent=2)
                for action_name in n_dict["d"]:
                    res += f" ({action_name})"
                    
                if n_dict["i"]:
                    res += "\n"
                    res += self._indentPrint(":b",indent=2)
                    for action_name in n_dict["i"]:
                        res += f" ({action_name})"
                res += ")\n"                        
        
        if self.debug:
            print(res)
        return res

def main(file_content,debug=True):
    p = QnpParser(debug=debug)
    p.loadQnp(file_content)
    for a in p.actions.values():
        # print(dir(a))
        print(
        a.action_name,
        a.action_preconditions,
        a.action_effects,
        a.increment_names,
        a.decrement_names)
    qp = QnpPddl(p,debug=debug)
    print(qp.outDomainPddl())
    print(qp.outProblemPddl())
    print(qp.outFairProblemPddl())



if __name__ == "__main__":
    import io
    file_str='''blocks-clear
2 n 1 holding 0
2 n 1 holding 0
1 n 0
4
Putaway
1 holding 1
1 holding 0
Pick-above-x
2 n 1 holding 0
2 n 0 holding 1
Put-above-x
1 holding 1
2 n 1 holding 0
Pick-other
1 holding 0
1 holding 1'''
    p = QnpParser(debug=True)
    p.loadQnp(io.StringIO(file_str))
    for a in p.actions.values():
        # print(dir(a))
        print(
        a.action_name,
        a.action_preconditions,
        a.action_effects,
        a.increment_names,
        a.decrement_names)

    # qp = QnpPddl(p,debug=True)
    qp = QnpPddl(p,debug=False)
    # qp.toPddl("/home/kali/Documents/domain.pddl","/home/kali/Documents/problem.pddl")
    print(qp.outDomainPddl())
    print(qp.outProblemPddl())
