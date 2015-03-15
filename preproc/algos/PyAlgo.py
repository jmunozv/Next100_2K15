from Centella.AAlgo import AAlgo
from Centella.physical_constants import *

class PyAlgo(AAlgo):

    def __init__(self,param=False,level = 1,label="",**kargs):

        """
        """
        
        self.name='PyAlgo'
        
        AAlgo.__init__(self,param,level,self.name,0,label,kargs)
        
        
        
    def initialize(self):

        """        
        """
    
        self.m.log(1,'+++Init method of PyAlgo algorithm+++')
        
        self.hman.h1("MyPyHisto","MyPyHisto",100,0,10)
        
        self.strings["MyPythonVar"]="ei"
        
        self.logman["USER"].strings["MyPythonVar2"]="ei"

        return



    def execute(self,event=""):

        """
  
        """
        
        self.m.log(1,'This is a histogram from C++Algo:')
        
        self.hman["MyC++Histo"].Print()
        
        cppalgo =  self.logman["CNTJob"].instances["C++Algo"]

        var = cppalgo.fetch_sstore("MyC++Var")
        
        self.m.log(1,'This is a parameter from C++Algo:',var)

        return True

        

    def finalize(self):

        self.m.log(1,'+++End method of PyAlgo algorithm+++')
        
        var = self.logman["C++Algo"].strings["MyC++Var"]
        
        self.m.log(1,'This is a parameter from C++Algo:',var)

        return
