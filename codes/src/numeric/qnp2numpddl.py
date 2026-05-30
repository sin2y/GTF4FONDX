from qnp2pddl import QnpParser,QnpPddl
import random

class QnpNumPddl(QnpPddl):
    def __init__(self,qnp : QnpParser,debug=False,num_init_values = [],increase_amount = 1,decrease_amount = 1) -> None:
        super().__init__(qnp,debug=False)
        self.increase_amount = increase_amount # (increase numerical_variable increase_amount) 
        self.decrease_amount = decrease_amount # (decrease numerical_variable decrease_amount)

        self.num_init_values_map = {}
        # random from 1 to 10
        if not num_init_values:
            for num_var in self.qnp.numeric_features:
                self.num_init_values_map[num_var] = random.randint(1,10)
        elif len(num_init_values) > 1:
            assert len(num_init_values) == len(self.qnp.numeric_features)
            for idx in range(len(num_init_values)):
                assert int(num_init_values[idx]) >= 0
                self.num_init_values_map[self.qnp.numeric_features[idx]] = int(num_init_values[idx]) 
        else:
            assert int(num_init_values[0]) <= 0
            for num_var in self.qnp.numeric_features:
                self.num_init_values_map[num_var] = -int(num_init_values[0])

        # self.domain_pddl = domain_pddl
        # self.problem_pddl = problem_pddl        
        

    def getFeatureLiteral(self,name,positive=True):
        '''old version:\n
        def getFeatureLiteral(self,name,positive=True):
            type = self.qnp.features[name]
            literal = f"({name}-gr0)" if type == 1 else f"({name})"
            if not positive:
                literal = f"(not {literal})"
            return literal
        '''
        f_type = self.qnp.features[name]
        literal = f"({name})"
        if f_type == 1: # numerical
            literal = f"(> {literal} 0)" if positive else f"(= {literal} 0)"
        else:
            if not positive:
                literal = f"(not {literal})"
        return literal
    

    def _outPredicates(self):
        res = self._indentPrint("(:predicates",indent=1)
        for name,type in self.qnp.features.items():
            # res += self._indentPrint(self.getFeatureLiteral(name),indent=2)
            if not type == 1: # boolean
                res += '\n'
                res += self._indentPrint(f"({name})",indent=2)
        res += ")\n\n"

        # numeric_features
        res += self._indentPrint("(:functions",indent=1)
        for name in self.qnp.numeric_features:
            res += '\n'
            # res += self._indentPrint(self.getFeatureLiteral(name),indent=2)
            res += self._indentPrint(f"({name})",indent=2)
        res += ")"

        if self.debug:
            print(res)

        return res     
        ...
    
    
    def _outActionPreconditions(self,action):
        '''return [ "(not (holding))", "(holding)", "(> (n) 0)", "(= (n) 0)"]'''
        res = []
        for name,val in action.action_preconditions:
            res.append(self.getFeatureLiteral(name,positive=val))
        return res
    
    def _outActionEffects(self,action):
        '''return [ "(not (holding))", "(holding)", "(increase (n) 1)", "(decrease (n) 1)"]'''
        res = []
        for name,val in action.action_effects:
            if name in self.qnp.numeric_features:
            # if self.qnp.features[name] == 1:
                if val == 1:
                    # inc
                    res.append(f"(increase ({name}) {self.increase_amount})")
                elif val == 0:
                    res.append(f"(decrease ({name}) {self.decrease_amount})")
            else:
                res.append(self.getFeatureLiteral(name,positive=val))
        return res
    
    def _outInit(self):
        res = ""
        res += self._indentPrint(f"(:init ",indent=1)
        one_time = True
        for name,val in self.qnp.inital:          
            if self.qnp.features[name] == 1: # numerical
                init_str = f"(= ({name}) {self.num_init_values_map[name]})" if val else f"(= ({name}) 0)"
                if one_time:
                    one_time = False
                    res += init_str
                    continue      
                res += '\n'
                res += self._indentPrint(init_str,indent=2)
            else: # boolean
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
        print(
        a.action_name,
        a.action_preconditions,
        a.action_effects,
        a.increment_names,
        a.decrement_names)

    qp = QnpNumPddl(p,debug=False)
    print(qp.outDomainPddl())
    print(qp.outProblemPddl())
