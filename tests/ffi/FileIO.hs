-- !!! Testing system calls

import Foreign
import Exception
import Prelude hiding (read)
import CTypes
import CString

tests = do
  putStrLn "\nTesting open, read and close"
  s <- testRead "ffi/FileIO.hs" 200
  putStrLn (map castCCharToChar s)

  putStrLn "\nTesting open, write and close"
  testWrite "/tmp/test_write" "Test successful"

foreign import ccall safe "static errno.h &errno" errno :: Ptr Int
        
withString0 s = bracket (newArray0 '\0' s) free
withBuffer sz m = do
  b <- mallocArray sz
  sz' <- m b
  s <- peekArray sz' b
  free b
  return s

foreign import ccall puts :: Ptr CChar -> IO Int


foreign import ccall "fcntl.h  open"  open'  :: Ptr CChar -> Int -> IO Int
foreign import ccall "fcntl.h  open"  open2' :: Ptr CChar -> Int -> Int -> IO Int
foreign import ccall "fcntl.h  creat" creat' :: Ptr CChar -> Int -> IO Int
foreign import ccall "unistd.h"       close  :: Int -> IO Int
foreign import ccall "unistd.h read"  read'  :: Int -> Ptr CChar -> Int -> IO Int
foreign import ccall "unistd.h write" write' :: Int -> Ptr CChar -> Int -> IO Int

creat s m   = withCString s $ \s' -> unix "creat" $ creat' s' m
open s m    = withCString s $ \s' -> unix "open"  $ open' s' m
open2 s m n = withCString s $ \s' -> unix "open2" $ open2' s' m n
write fd s  = withCString s $ \s' -> unix "write" $ write' fd s' (length s)
read  fd sz = withBuffer sz $ \s' -> unix "read"  $ read' fd s' sz

unix s m = do
  x <- m
  if x < 0 
   then do
     err <- peek errno
     ioError $ userError $ s ++ ": " ++ show (x,err)
   else return x

testRead fn sz = bracket (open fn 0) close (flip read sz)
testWrite fn s = bracket (open2 fn (512+64+1) 511) close (flip write s)

