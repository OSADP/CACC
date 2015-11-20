function veh_pos(block)
%MSFUNTMPL_BASIC A Template for a Level-2 MATLAB S-Function
%   The MATLAB S-function is written as a MATLAB function with the
%   same name as the S-function. Replace 'msfuntmpl_basic' with the 
%   name of your S-function.
%
%   It should be noted that the MATLAB S-function is very similar
%   to Level-2 C-Mex S-functions. You should be able to get more
%   information for each of the block methods by referring to the
%   documentation for C-Mex S-functions.
%
%   Copyright 2003-2010 The MathWorks, Inc.

%%
%% The setup method is used to set up the basic attributes of the
%% S-function such as ports, parameters, etc. Do not add any other
%% calls to the main body of the function.
%%
setup(block);

%endfunction

%% Function: setup ===================================================
%% Abstract:
%%   Set up the basic characteristics of the S-function block such as:
%%   - Input ports
%%   - Output ports
%%   - Dialog parameters
%%   - Options
%%
%%   Required         : Yes
%%   C-Mex counterpart: mdlInitializeSizes
%%
function setup(block)

% Register number of ports
block.NumInputPorts  = 6;
block.NumOutputPorts = 1;

% Setup port properties to be inherited or dynamic
block.SetPreCompInpPortInfoToDynamic;
block.SetPreCompOutPortInfoToDynamic;

% Override input port properties
block.InputPort(1).Dimensions        = 1;
block.InputPort(1).DatatypeID  = 0;  % double
block.InputPort(1).Complexity  = 'Real';
block.InputPort(1).DirectFeedthrough = true;

block.InputPort(2).Dimensions        = 3;
block.InputPort(2).DatatypeID  = 0;  % double
block.InputPort(2).Complexity  = 'Real';
block.InputPort(2).DirectFeedthrough = true;

block.InputPort(3).Dimensions        = 3;
block.InputPort(3).DatatypeID  = 0;  % double
block.InputPort(3).Complexity  = 'Real';
block.InputPort(3).DirectFeedthrough = true;

block.InputPort(4).Dimensions        = 3;
block.InputPort(4).DatatypeID  = 0;  % double
block.InputPort(4).Complexity  = 'Real';
block.InputPort(4).DirectFeedthrough = true;

block.InputPort(5).Dimensions       = 3;
block.InputPort(5).DatatypeID  = 0; % double
block.InputPort(5).Complexity  = 'Real';
block.InputPort(5).DirectFeedthrough = true;

block.InputPort(6).Dimensions        = 3;
block.InputPort(6).DatatypeID  = 0;  % double
block.InputPort(6).Complexity  = 'Real';
block.InputPort(6).DirectFeedthrough = true;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
block.OutputPort(1).Dimensions        = 15;
block.OutputPort(1).DatatypeID  = 0;  % double
block.OutputPort(1).Complexity  = 'Real';


% Register parameters
block.NumDialogPrms     = 0;

% Register sample times
%  [0 offset]            : Continuous sample time
%  [positive_num offset] : Discrete sample time
%
%  [-1, 0]               : Inherited sample time
%  [-2, 0]               : Variable sample time
block.SampleTimes = [0 0];

% Specify the block simStateCompliance. The allowed values are:
%    'UnknownSimState', < The default setting; warn and assume DefaultSimState
%    'DefaultSimState', < Same sim state as a built-in block
%    'HasNoSimState',   < No sim state
%    'CustomSimState',  < Has GetSimState and SetSimState methods
%    'DisallowSimState' < Error out when saving or restoring the model sim state
block.SimStateCompliance = 'DefaultSimState';

%% -----------------------------------------------------------------
%% The MATLAB S-function uses an internal registry for all
%% block methods. You should register all relevant methods
%% (optional and required) as illustrated below. You may choose
%% any suitable name for the methods and implement these methods
%% as local functions within the same file. See comments
%% provided for each function for more information.
%% -----------------------------------------------------------------

block.RegBlockMethod('PostPropagationSetup',    @DoPostPropSetup);
block.RegBlockMethod('InitializeConditions', @InitializeConditions);
block.RegBlockMethod('Start', @Start);
block.RegBlockMethod('Outputs', @Outputs);     % Required
block.RegBlockMethod('Update', @Update);
block.RegBlockMethod('Derivatives', @Derivatives);
block.RegBlockMethod('Terminate', @Terminate); % Required

%end setup

%%
%% PostPropagationSetup:
%%   Functionality    : Setup work areas and state variables. Can
%%                      also register run-time methods here
%%   Required         : No
%%   C-Mex counterpart: mdlSetWorkWidths
%%
function DoPostPropSetup(block)
block.NumDworks = 1;
  
  block.Dwork(1).Name            = 'x1';
  block.Dwork(1).Dimensions      = 1;
  block.Dwork(1).DatatypeID      = 0;      % double
  block.Dwork(1).Complexity      = 'Real'; % real
  block.Dwork(1).UsedAsDiscState = true;


%%
%% InitializeConditions:
%%   Functionality    : Called at the start of simulation and if it is 
%%                      present in an enabled subsystem configured to reset 
%%                      states, it will be called when the enabled subsystem
%%                      restarts execution to reset the states.
%%   Required         : No
%%   C-MEX counterpart: mdlInitializeConditions
%%
function InitializeConditions(block)

%end InitializeConditions


%%
%% Start:
%%   Functionality    : Called once at start of model execution. If you
%%                      have states that should be initialized once, this 
%%                      is the place to do it.
%%   Required         : No
%%   C-MEX counterpart: mdlStart
%%
function Start(block)

block.Dwork(1).Data = 0;

% end Start

%%
%% Outputs:
%%   Functionality    : Called to generate block outputs in
%%                      simulation step
%%   Required         : Yes
%%   C-MEX counterpart: mdlOutputs
%%
function Outputs(block)

    %% to get input signals
    
    %%N_veh = block.InputPort(1, 0).Data;
    tmp = block.InputPort(1).Data;
    cacc_grp_len=tmp(1);    
    bsm_info_1=block.InputPort(2).Data;
    bsm_info_2=block.InputPort(3).Data;
    bsm_info_3=block.InputPort(4).Data;
    bsm_info_4=block.InputPort(5).Data;
    bsm_info_5=block.InputPort(6).Data;

    
    
        if (cacc_grp_len == 2)                %% always put the subject vehicle at Virtual Position 1 in the Vector Forming            
                A=[bsm_info_1(2); bsm_info_2(2)];
                b=[bsm_info_1(3); bsm_info_2(3)];            
        elseif (cacc_grp_len == 3)            
                A=[bsm_info_1(2); bsm_info_2(2); bsm_info_3(2)];
                b=[bsm_info_1(3); bsm_info_2(3); bsm_info_3(3)];           
        elseif (cacc_grp_len == 4)           
                A=[bsm_info_1(2); bsm_info_2(2); bsm_info_3(2); bsm_info_4(2)];
                b=[bsm_info_1(3); bsm_info_2(3); bsm_info_3(3); bsm_info_4(3)];            
        elseif (cacc_grp_len == 5)            
                A=[bsm_info_1(2); bsm_info_2(2); bsm_info_3(2); bsm_info_4(2); bsm_info_5(2)];
                b=[bsm_info_1(3); bsm_info_2(3); bsm_info_3(3); bsm_info_4(3); bsm_info_5(3)];           
        else
        end
    
    a=A\b;
 
    alpha=atan(a(1));    %% angle of moving axle w.r.t. to East of the original coordinate system
    
    alpha=pi/2.0-alpha;  %% angle of moving axle w.r.t. to North of the original coordinate system
    alpha=-alpha;        %% so that the original y-axis is in the Miving Axle direction;
    
    if  cacc_grp_len == 1
            block.OutputPort(1).Data(1)=bsm_info_1(1); %%BSM1: ID
            block.OutputPort(1).Data(2)=1; %%BSM1: Contol Pos
            block.OutputPort(1).Data(3)=0; %%BSM1: Lane ID
            block.OutputPort(1).Data(4)=-1; %%BSM2: ID
            block.OutputPort(1).Data(5)=-1; %%BSM2: Contol Pos
            block.OutputPort(1).Data(6)=-1; %%BSM2: Lane ID
            block.OutputPort(1).Data(7)=-1; %%BSM3: ID
            block.OutputPort(1).Data(8)=-1; %%BSM3: Contol Pos
            block.OutputPort(1).Data(9)=-1; %%BSM3: Lane ID
            block.OutputPort(1).Data(10)=-1; %%BSM4: ID
            block.OutputPort(1).Data(11)=-1; %%BSM4: Contol Pos
            block.OutputPort(1).Data(12)=-1; %%BSM4: Lane ID
            block.OutputPort(1).Data(13)=-1; %%BSM5: ID
            block.OutputPort(1).Data(14)=-1; %%BSM5: Contol Pos
            block.OutputPort(1).Data(15)=-1; %%BSM5: Lane ID
    elseif (cacc_grp_len == 2)                %% always put the subject vehicle at Virtual Position 1 in the Vector Forming
            
            p1_2=Coord_Trans(alpha,A(1),b(1),A(2),b(2));
            tmp=[0 p1_2(2)];
            [Ord, Ind]=sort(tmp, 'descend');
            
%             if subj_veh_Id == bsm_info_1(1)
%                subj_veh_control_pos=Ord(1); 
%             elseif subj_veh_Id == bsm_info_2(1)
%                subj_veh_control_pos=Ord(2); 
%             else
%             end
				  block.OutputPort(1).Data(1)=bsm_info_1(1); %%BSM1: ID
				  block.OutputPort(1).Data(2)=Ind(1); %%BSM1: Contol Pos
				  block.OutputPort(1).Data(3)=0; %%BSM1: Lane ID
				  block.OutputPort(1).Data(4)=bsm_info_2(1); %%BSM2: ID
				  block.OutputPort(1).Data(5)=Ind(2); %%BSM2: Contol Pos
				  block.OutputPort(1).Data(6)=0; %%BSM2: Lane ID
				  block.OutputPort(1).Data(7)=-1; %%BSM3: ID
				  block.OutputPort(1).Data(8)=-1; %%BSM3: Contol Pos
				  block.OutputPort(1).Data(9)=-1; %%BSM3: Lane ID
				  block.OutputPort(1).Data(10)=-1; %%BSM4: ID
				  block.OutputPort(1).Data(11)=-1; %%BSM4: Contol Pos
				  block.OutputPort(1).Data(12)=-1; %%BSM4: Lane ID
				  block.OutputPort(1).Data(13)=-1; %%BSM5: ID
				  block.OutputPort(1).Data(14)=-1; %%BSM5: Contol Pos
				  block.OutputPort(1).Data(15)=-1; %%BSM5: Lane ID
        elseif (cacc_grp_len == 3)
            p1_2=Coord_Trans(alpha,A(1),b(1),A(2),b(2));
            p1_3=Coord_Trans(alpha,A(1),b(1),A(3),b(3));
            tmp=[0 p1_2(2) p1_3(2)];
            [Ord, Ind]=sort(tmp, 'descend');
            
block.OutputPort(1).Data(1)=bsm_info_1(1); %%BSM1: ID
block.OutputPort(1).Data(2)=Ind(1); %%BSM1: Contol Pos
block.OutputPort(1).Data(3)=0; %%BSM1: Lane ID
block.OutputPort(1).Data(4)=bsm_info_2(1); %%BSM2: ID
block.OutputPort(1).Data(5)=Ind(2); %%BSM2: Contol Pos
block.OutputPort(1).Data(6)=0; %%BSM2: Lane ID
block.OutputPort(1).Data(7)=bsm_info_3(1); %%BSM3: ID
block.OutputPort(1).Data(8)=Ind(3); %%BSM3: Contol Pos
block.OutputPort(1).Data(9)=0; %%BSM3: Lane ID
block.OutputPort(1).Data(10)=-1; %%BSM4: ID
block.OutputPort(1).Data(11)=-1; %%BSM4: Contol Pos
block.OutputPort(1).Data(12)=-1; %%BSM4: Lane ID
block.OutputPort(1).Data(13)=-1; %%BSM5: ID
block.OutputPort(1).Data(14)=-1; %%BSM5: Contol Pos
block.OutputPort(1).Data(15)=-1; %%BSM5: Lane ID
            

          
        elseif (cacc_grp_len == 4)
            p1_2=Coord_Trans(alpha,A(1),b(1),A(2),b(2));
            p1_3=Coord_Trans(alpha,A(1),b(1),A(3),b(3));
            p1_4=Coord_Trans(alpha,A(1),b(1),A(4),b(4));
            tmp=[0 p1_2(2) p1_3(2) p1_4(2)];
            [Ord, Ind]=sort(tmp, 'descend');
            
block.OutputPort(1).Data(1)=bsm_info_1(1); %%BSM1: ID
block.OutputPort(1).Data(2)=Ind(1); %%BSM1: Contol Pos
block.OutputPort(1).Data(3)=0; %%BSM1: Lane ID
block.OutputPort(1).Data(4)=bsm_info_2(1); %%BSM2: ID
block.OutputPort(1).Data(5)=Ind(2); %%BSM2: Contol Pos
block.OutputPort(1).Data(6)=0; %%BSM2: Lane ID
block.OutputPort(1).Data(7)=bsm_info_3(1); %%BSM3: ID
block.OutputPort(1).Data(8)=Ind(3); %%BSM3: Contol Pos
block.OutputPort(1).Data(9)=0; %%BSM3: Lane ID
block.OutputPort(1).Data(10)=bsm_info_4(1); %%BSM4: ID
block.OutputPort(1).Data(11)=Ind(4); %%BSM4: Contol Pos
block.OutputPort(1).Data(12)=0; %%BSM4: Lane ID
block.OutputPort(1).Data(13)=-1; %%BSM5: ID
block.OutputPort(1).Data(14)=-1; %%BSM5: Contol Pos
block.OutputPort(1).Data(15)=-1; %%BSM5: Lane ID
                
        elseif (cacc_grp_len == 5)
            p1_2=Coord_Trans(alpha,A(1),b(1),A(2),b(2));
            p1_3=Coord_Trans(alpha,A(1),b(1),A(3),b(3));
            p1_4=Coord_Trans(alpha,A(1),b(1),A(4),b(4));
            p1_5=Coord_Trans(alpha,A(1),b(1),A(5),b(5));
            tmp=[0 p1_2(2) p1_3(2) p1_4(2) p1_5(2)];            
            [Ord, Ind]=sort(tmp, 'descend');
            
block.OutputPort(1).Data(1)=bsm_info_1(1); %%BSM1: ID
block.OutputPort(1).Data(2)=Ind(1); %%BSM1: Contol Pos
block.OutputPort(1).Data(3)=0; %%BSM1: Lane ID
block.OutputPort(1).Data(4)=bsm_info_2(1); %%BSM2: ID
block.OutputPort(1).Data(5)=Ind(2); %%BSM2: Contol Pos
block.OutputPort(1).Data(6)=0; %%BSM2: Lane ID
block.OutputPort(1).Data(7)=bsm_info_3(1); %%BSM3: ID
block.OutputPort(1).Data(8)=Ind(3); %%BSM3: Contol Pos
block.OutputPort(1).Data(9)=0; %%BSM3: Lane ID
block.OutputPort(1).Data(10)=bsm_info_4(1); %%BSM4: ID
block.OutputPort(1).Data(11)=Ind(4); %%BSM4: Contol Pos
block.OutputPort(1).Data(12)=0; %%BSM4: Lane ID
block.OutputPort(1).Data(13)=bsm_info_4(1); %%BSM5: ID
block.OutputPort(1).Data(14)=Ind(5); %%BSM5: Contol Pos
block.OutputPort(1).Data(15)=0; %%BSM5: Lane ID
        else
        end



%block.OutputPort(1).Data = block.Dwork(1).Data + block.InputPort(1).Data;

% end Outputs

%%
%% Update:
%%   Functionality    : Called to update discrete states
%%                      during simulation step
%%   Required         : No
%%   C-MEX counterpart: mdlUpdate
%%
function Update(block)

block.Dwork(1).Data = block.InputPort(1).Data;

% end Update

%%
%% Derivatives:
%%   Functionality    : Called to update derivatives of
%%                      continuous states during simulation step
%%   Required         : No
%%   C-MEX counterpart: mdlDerivatives
%
function Derivatives(block)

% end Derivatives

%%
%% Terminate:
%%   Functionality    : Called at the end of simulation for cleanup
%%   Required         : Yes
%%   C-MEX counterpart: mdlTerminate
%%
function Terminate(block)

% end Terminate

function local  = Coord_Trans(alpha, subj_x, subj_y, x, y)

    alpha=pi/2.0-alpha;  %% angle of moving axle w.r.t. to North of the original coordinate system
    alpha=-alpha;        %% so that the original y-axis is in the Miving Axle direction;
    
    R=[cos(alpha) sin(alpha); -sin(alpha) cos(alpha)];
    
    p1=[subj_x; subj_y];  %% coordinate of p1 in old system
    p2=[x;y];  %% coordinate of p2 in old system
  
  local = R*(p2-p1);   
% end  Coord_Trans


