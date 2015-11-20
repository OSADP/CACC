classdef AccelCommandMode < Simulink.IntEnumType
	enumeration
		ACC_Disable(0)
		WrenchEffort(1)
		SpeedControl(2)
	end
	methods (Static = true)
		function retVal = getDescription()
			retVal = 'Acceration Command Modes';
		end
		function retVal = getDefaultValue()
			retVal = AccelCommandMode.ACC_Disable;
        end
		function retVal = addClassNameToEnumNames()
			retVal = true;
		end
	end
end