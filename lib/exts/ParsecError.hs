----------------------------------------------------------------
-- Daan Leijen (c) 1999-2001, daan@cs.uu.nl
--
-- Parse errors
--
-- $Revision: 1.1 $
-- $Author: ross $
-- $Date: 2002/09/03 13:59:49 $
----------------------------------------------------------------
module ParsecError( Message(SysUnExpect,UnExpect,Expect,Message)
                  , messageString, messageCompare, messageEq
                  
                  , ParseError, errorPos, errorMessages, errorIsUnknown
                  , showErrorMessages
                  
                  , newErrorMessage, newErrorUnknown
                  , addErrorMessage, setErrorPos, setErrorMessage
                  , mergeError
                  )
                  where


import List      (nub,sortBy)
import ParsecPos 
                          
-----------------------------------------------------------
-- Messages
-----------------------------------------------------------                         
data Message        = SysUnExpect !String   --library generated unexpect            
                    | UnExpect    !String   --unexpected something     
                    | Expect      !String   --expecting something
                    | Message     !String   --raw message
                    
messageToEnum msg
    = case msg of SysUnExpect _ -> 0
                  UnExpect _    -> 1
                  Expect _      -> 2
                  Message _     -> 3                                  
                                      
messageCompare msg1 msg2
    = compare (messageToEnum msg1) (messageToEnum msg2)
  
messageString msg
    = case msg of SysUnExpect s -> s
                  UnExpect s    -> s
                  Expect s      -> s
                  Message s     -> s                                  

messageEq msg1 msg2
    = (messageCompare msg1 msg2 == EQ)
    
    
-----------------------------------------------------------
-- Parse Errors
-----------------------------------------------------------                           
data ParseError     = ParseError !SourcePos [Message]

errorPos :: ParseError -> SourcePos
errorPos (ParseError pos msgs)
    = pos
                  
errorMessages :: ParseError -> [Message]
errorMessages (ParseError pos msgs)
    = sortBy messageCompare msgs      
        
errorIsUnknown :: ParseError -> Bool
errorIsUnknown (ParseError pos msgs)
    = null msgs
            
            
-----------------------------------------------------------
-- Create parse errors
-----------------------------------------------------------                         
newErrorUnknown pos
    = ParseError pos []
    
newErrorMessage msg pos  
    = ParseError pos [msg]

addErrorMessage msg (ParseError pos msgs)
    = ParseError pos (msg:msgs)
    
setErrorPos pos (ParseError _ msgs)
    = ParseError pos msgs
    
setErrorMessage msg (ParseError pos msgs)
    = ParseError pos (msg:filter (not . messageEq msg) msgs)
 
    
mergeError :: ParseError -> ParseError -> ParseError
mergeError (ParseError pos msgs1) (ParseError _ msgs2)
    = ParseError pos (msgs1 ++ msgs2)
    


-----------------------------------------------------------
-- Show Parse Errors
-----------------------------------------------------------                         
instance Show ParseError where
  show err
    = show (errorPos err) ++ ":" ++ 
      showErrorMessages "or" "unknown parse error" 
                        "expecting" "unexpected" "end of input"
                       (errorMessages err)


-- Language independent show function
showErrorMessages msgOr msgUnknown msgExpecting msgUnExpected msgEndOfInput msgs
    | null msgs = msgUnknown
    | otherwise = concat $ map ("\n"++) $ clean $
                 [showSysUnExpect,showUnExpect,showExpect,showMessages]
    where
      (sysUnExpect,msgs1)   = span (messageEq (SysUnExpect "")) msgs
      (unExpect,msgs2)      = span (messageEq (UnExpect "")) msgs1
      (expect,messages)     = span (messageEq (Expect "")) msgs2
    
      showExpect        = showMany msgExpecting expect
      showUnExpect      = showMany msgUnExpected unExpect
      showSysUnExpect   | not (null unExpect) ||
                          null sysUnExpect       = ""
                        | null firstMsg          = msgUnExpected ++ " " ++ msgEndOfInput
                        | otherwise              = msgUnExpected ++ " " ++ firstMsg
                        where
                          firstMsg  = messageString (head sysUnExpect)
                        
      showMessages      = showMany "" messages

      
      --helpers                                                                                                                                        
      showMany pre msgs = case (clean (map messageString msgs)) of
                            [] -> ""
                            ms | null pre  -> commasOr ms
                               | otherwise -> pre ++ " " ++ commasOr ms
                            
      commasOr []       = ""                
      commasOr [m]      = m                
      commasOr ms       = commaSep (init ms) ++ " " ++ msgOr ++ " " ++ last ms
        
      commaSep          = seperate ", " . clean
      semiSep           = seperate "; " . clean       
        
      seperate sep []   = ""
      seperate sep [m]  = m
      seperate sep (m:ms) = m ++ sep ++ seperate sep ms                            
      
      clean             = nub . filter (not.null)                  
      
