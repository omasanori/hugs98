--!!! Testing the MVar primitives

module TestMVar(test1,test2,test3,test4,test5,test6,test7,test8) where

-- should print "a"
test1 =
  newMVar         >>= \ v ->
  putMVar v 'a'   >>
  takeMVar v      >>= \ x ->
  putChar x       >>
  takeMVar v      >>= \ x ->
  putChar x

-- Nondeterministic
test2 = 
  newMVar         >>= \ v ->
  forkIO (p1 v)   >>
  get v           >>
  put v 'b'
 where
  p1 v = 
   put v 'a'      >>
   get v

-- should print "a"
test3 = 
  newMVar              >>= \ v ->
  forkIO (put v 'a')   >>
  get v

-- should print "ab"   
test4 = 
  newMVar         >>= \ v1 ->
  newMVar         >>= \ v2 ->
  forkIO (p1 v1 v2)   >>
  get v1          >>
  put v2 'b'
 where
  p1 v1 v2 = 
   put v1 'a'     >>
   get v2

-- should abort: primPutMVar: full MVar
test5 = 
  newMVar         >>= \ v ->
  put v 'a'       >>
  put v 'b'

-- Tests blocking of two processes on the same variable.
-- should print "aa"
test6 = 
  newMVar         			>>= \ ack ->
  newMVar         			>>= \ a1 ->
  newMVar         			>>= \ a2 ->
  forkIO (putMVar a1 () >> get ack)  	>>
  forkIO (putMVar a2 () >> get ack)  	>>
  takeMVar a2     			>> 
  takeMVar a1     			>>
  put ack 'a'     			>>
  put ack 'a'

----------------------------------------------------------------
-- Non-deterministic tests below this point
-- Must be tested interactively and probably don't work using 
-- "logical concurrency".


-- should print interleaving of a's and b's
-- (degree of interleaving depends on granularity of concurrency)
test7 =
  forkIO a >> b
 where
  a = putStr "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
  b = putStr "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"

-- should give infinite interleaving of a's and b's
-- (degree of interleaving depends on granularity of concurrency)
-- Ming's example.  The Hugs read-eval-print loop gets confused if 
-- there's no type signature
test8 :: IO ()
test8 =
  forkIO a >> b
 where
  -- symbols carefully chosen to make them look very different on screen
  a = putChar 'a' >> a
  b = putChar 'B' >> b

-- Tests blocking of two processes on the same variable.
-- may print "ab" or "ba"
test9 = 
  newMVar         			>>= \ ack ->
  newMVar         			>>= \ a1 ->
  newMVar         			>>= \ a2 ->
  forkIO (putMVar a1 () >> get ack)  	>>
  forkIO (putMVar a2 () >> get ack)  	>>
  takeMVar a2     			>> 
  takeMVar a1     			>>
  put ack 'a'     			>>
  put ack 'b'

put v x =
  putMVar v x

get v =
  takeMVar v      >>= \ x ->
  putChar x
