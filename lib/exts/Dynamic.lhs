%
% (c) AQUA Project, Glasgow University, 1998
%

Cheap and cheerful dynamic types.

The Dynamic interface is part of the Hugs/GHC standard
libraries, providing basic support for dynamic types.

Operations for injecting values of arbitrary type into
a dynamically typed value, Dynamic, are provided, together
with operations for converting dynamic values into a concrete
(monomorphic) type.

The Dynamic implementation provided is closely based on code
contained in Hugs library of the same name.

NOTE: test code at the end, but commented out.

\begin{code}
module Dynamic
    (
      -- dynamic type
      Dynamic	  -- abstract, instance of: Show (?)
    , toDyn       -- :: Typeable a => a -> Dynamic
    , fromDyn	  -- :: Typeable a => Dynamic -> a -> a
    , fromDynamic -- :: Typeable a => Dynamic -> Maybe a
	
      -- type representation

    , Typeable(typeOf) 
      -- class Typeable a where { typeOf :: a -> TypeRep }

      -- Dynamic defines Typeable instances for the following
      -- Prelude types: Char, Int, Float, Double, Bool
      --                (), Maybe a, (a->b), [a]
      --		(a,b) (a,b,c) (a,b,c,d) (a,b,c,d,e)

    , TypeRep      -- abstract, instance of: Eq, Show
    , TyCon        -- abstract, instance of: Eq, Show

      -- type representation constructors/operators:
    , mkTyCon	   -- :: String  -> TyCon
    , mkAppTy	   -- :: TyCon   -> [TypeRep] -> TypeRep
    , mkFunTy      -- :: TypeRep -> TypeRep   -> TypeRep
    , applyTy	   -- :: TypeRep -> TypeRep   -> Maybe TypeRep

      -- 
      -- let iTy = mkTyCon "Int" in show (mkAppTy (mkTyCon ",,")
      --                                 [iTy,iTy,iTy])
      -- 
      -- returns "(Int,Int,Int)"
      --
      -- The TypeRep Show instance promises to print tuple types
      -- correctly. Tuple type constructors are specified by a 
      -- sequence of commas, e.g., (mkTyCon ",,,,,,") returns
      -- the 7-tuple tycon.
    ) where

{- BEGIN_FOR_GHC
import GlaExts
import PrelDynamic
   END_FOR_GHC -}

import IOExts 
       ( unsafePerformIO,
         IORef, newIORef, readIORef, writeIORef
        )

{- BEGIN_FOR_HUGS -}
data TypeRep
 = App TyCon   [TypeRep]
 | Fun TypeRep TypeRep
   deriving ( Eq )

-- type constructors are 
data TyCon = TyCon Int String

instance Eq TyCon where
  (TyCon t1 _) == (TyCon t2 _) = t1 == t2

data Dynamic = Dynamic TypeRep Obj

data Obj = Obj  
 -- dummy type to hold the dynamically typed value.

primitive unsafeCoerce "primUnsafeCoerce" :: a -> b
{- END_FOR_HUGS -}

{- BEGIN_FOR_GHC
unsafeCoerce :: a -> b
unsafeCoerce = unsafeCoerce#
   END_FOR_GHC -}
\end{code}

The dynamic type is represented by Dynamic, carrying
the dynamic value along with its type representation:

\begin{code}
-- the instance just prints the type representation.
instance Show Dynamic where
   showsPrec _ (Dynamic t _) = 
          showString "<<" . 
	  showsPrec 0 t   . 
	  showString ">>"
\end{code}

Operations for going to and from Dynamic:

\begin{code}
toDyn :: Typeable a => a -> Dynamic
toDyn v = Dynamic (typeOf v) (unsafeCoerce v)

fromDyn :: Typeable a => Dynamic -> a -> a
fromDyn (Dynamic t v) def
  | typeOf def == t = unsafeCoerce v
  | otherwise       = def

fromDynamic :: Typeable a => Dynamic -> Maybe a
fromDynamic (Dynamic t v) =
  case unsafeCoerce v of 
    r | t == typeOf r -> Just r
      | otherwise     -> Nothing
\end{code}

(Abstract) universal datatype:

\begin{code}
instance Show TypeRep where
  showsPrec p (App tycon tys) =
    case tys of
      [] -> showsPrec p tycon
      [x] | tycon == listTc    -> showChar '[' . shows x . showChar ']'
      xs  
        | isTupleTyCon tycon -> showTuple tycon xs
	| otherwise	     ->
	    showParen (p > 9) $
   	    showsPrec p tycon . 
	    showChar ' '      . 
	    showArgs tys

  showsPrec p (Fun f a) =
     showParen (p > 8) $
     showsPrec 9 f . showString " -> " . showsPrec 8 a
\end{code}

To make it possible to convert values with user-defined types
into type Dynamic, we need a systematic way of getting
the type representation of an arbitrary type. Type class
provide a good fit, here

\begin{code}
class Typeable a where
  typeOf :: a -> TypeRep
\end{code}

NOTE: The argument to the overloaded `typeOf' is only
used to carry type information, and Typeable instances
should *never* look at its value.

\begin{code}
isTupleTyCon :: TyCon -> Bool
isTupleTyCon (TyCon _ (',':_)) = True
isTupleTyCon _		       = False

instance Show TyCon where
  showsPrec _ (TyCon _ s) = showString s

-- 
-- If we enforce the restriction that TyCons are
-- shared, we can map them onto Ints very simply
-- which allows for efficient comparison.
--
mkTyCon :: String -> TyCon
mkTyCon str = unsafePerformIO $ do
   v <- readIORef uni
   writeIORef uni (v+1)
   return (TyCon v str)

uni :: IORef Int
uni = unsafePerformIO ( newIORef 0 )
\end{code}

Some (Show.TypeRep) helpers:

\begin{code}
showArgs :: Show a => [a] -> ShowS
showArgs [] = id
showArgs [a] = showsPrec 10 a
showArgs (a:as) = showsPrec 10 a . showString " " . showArgs as 

showTuple :: TyCon -> [TypeRep] -> ShowS
showTuple (TyCon _ str) args = showChar '(' . go str args
 where
  go [] [a] = showsPrec 10 a . showChar ')'
  go _  []  = showChar ')' -- a failure condition, really.
  go (',':xs) (a:as) = showsPrec 10 a . showChar ',' . go xs as
  go _ _   = showChar ')'
\end{code}

\begin{code}
mkAppTy  :: TyCon   -> [TypeRep] -> TypeRep
mkAppTy tyc args = App tyc args

mkFunTy  :: TypeRep -> TypeRep   -> TypeRep
mkFunTy f a = Fun f a
\end{code}

Auxillary functions

\begin{code}
-- (f::(a->b)) `dynApply` (x::a) = (f a)::b
dynApply :: Dynamic -> Dynamic -> Maybe Dynamic
dynApply (Dynamic t1 f) (Dynamic t2 x) =
  case applyTy t1 t2 of
    Just t3 -> Just (Dynamic t3 ((unsafeCoerce f) x))
    Nothing -> Nothing

dynApp :: Dynamic -> Dynamic -> Dynamic
dynApp f x = case dynApply f x of 
             Just r -> r
             Nothing -> error ("Type error in dynamic application.\n" ++
                               "Can't apply function " ++ show f ++
                               " to argument " ++ show x)

applyTy :: TypeRep -> TypeRep -> Maybe TypeRep
applyTy (Fun t1 t2) t3
  | t1 == t3    = Just t2
applyTy _ _     = Nothing

\end{code}

\begin{code}
instance Typeable Int where
  typeOf _ = mkAppTy intTc []
  
instance Typeable Char where
  typeOf _ = mkAppTy charTc []
  
instance Typeable Bool where
  typeOf _ = mkAppTy boolTc []
  
instance Typeable Float where
  typeOf _ = mkAppTy floatTc []
  
instance Typeable Double where
  typeOf _ = mkAppTy doubleTc []

instance Typeable Integer where
  typeOf _ = mkAppTy integerTc []

instance Typeable a => Typeable (IO a) where
  typeOf action = mkAppTy ioTc [typeOf (doIO action)]
    where
      doIO :: IO a -> a
      doIO = undefined

instance Typeable a => Typeable [a] where
  typeOf ls = mkAppTy listTc [typeOf (hd ls)]
    where
      hd :: [a] -> a
      hd = undefined

instance Typeable a => Typeable (Maybe a) where
  typeOf mb = mkAppTy maybeTc [typeOf (getJ mb)]
    where
      getJ :: Maybe a -> a
      getJ = undefined

instance (Typeable a, Typeable b) => Typeable (Either a b) where
  typeOf ei = mkAppTy maybeTc [typeOf (getL ei), typeOf (getR ei)]
    where
      getL :: Either a b -> a
      getL = undefined
      getR :: Either a b -> a
      getR = undefined

instance (Typeable a, Typeable b) => Typeable (a -> b) where
  typeOf f = mkFunTy (typeOf (arg f)) (typeOf (res f))
   where
    arg :: (a -> b) -> a
    arg = undefined
    
    res :: (a -> b) -> b
    res = undefined

instance Typeable () where
  typeOf _ = mkAppTy unitTc []

instance Typeable TypeRep where
  typeOf _ = mkAppTy typeRepTc []

instance Typeable TyCon where
  typeOf _ = mkAppTy tyConTc []

instance Typeable Dynamic where
  typeOf _ = mkAppTy dynamicTc []

instance Typeable Ordering where
  typeOf _ = mkAppTy orderingTc []

instance (Typeable a, Typeable b) => Typeable (a,b) where
  typeOf tu = mkAppTy tup2Tc [typeOf (fst tu), typeOf (snd tu)]
    where
      fst :: (a,b) -> a
      fst = undefined
      snd :: (a,b) -> b
      snd = undefined

      tup2Tc = mkTyCon ","

instance ( Typeable a
         , Typeable b
	 , Typeable c) => Typeable (a,b,c) where
  typeOf tu = mkAppTy tup3Tc [ typeOf (fst tu)
                             , typeOf (snd tu)
			     , typeOf (thd tu)
			     ]
    where
      fst :: (a,b,c) -> a
      fst = undefined
      snd :: (a,b,c) -> b
      snd = undefined
      thd :: (a,b,c) -> c
      thd = undefined

      tup3Tc = mkTyCon ",,"

instance ( Typeable a
	 , Typeable b
	 , Typeable c
	 , Typeable d) => Typeable (a,b,c,d) where
  typeOf tu = mkAppTy tup4Tc [ typeOf (fst tu)
                             , typeOf (snd tu)
			     , typeOf (thd tu)
			     , typeOf (fth tu)
			     ]
    where
      fst :: (a,b,c,d) -> a
      fst = undefined
      snd :: (a,b,c,d) -> b
      snd = undefined
      thd :: (a,b,c,d) -> c
      thd = undefined
      fth :: (a,b,c,d) -> d
      fth = undefined

      tup4Tc = mkTyCon ",,,"

instance ( Typeable a
	 , Typeable b
	 , Typeable c
	 , Typeable d
	 , Typeable e) => Typeable (a,b,c,d,e) where
  typeOf tu = mkAppTy tup5Tc [ typeOf (fst tu)
                             , typeOf (snd tu)
			     , typeOf (thd tu)
			     , typeOf (fth tu)
			     , typeOf (ffth tu)
			     ]
    where
      fst :: (a,b,c,d,e) -> a
      fst = undefined
      snd :: (a,b,c,d,e) -> b
      snd = undefined
      thd :: (a,b,c,d,e) -> c
      thd = undefined
      fth :: (a,b,c,d,e) -> d
      fth = undefined
      ffth :: (a,b,c,d,e) -> e
      ffth = undefined

      tup5Tc = mkTyCon ",,,,"

\end{code}

@TyCon@s are provided for the following:

\begin{code}
-- prelude types:
intTc, charTc, boolTc :: TyCon
intTc      = mkTyCon "Int"
charTc     = mkTyCon "Char"
boolTc     = mkTyCon "Bool"

floatTc, doubleTc, integerTc :: TyCon
floatTc    = mkTyCon "Float"
doubleTc   = mkTyCon "Double"
integerTc  = mkTyCon "Integer"

ioTc, maybeTc, eitherTc, listTc :: TyCon
ioTc       = mkTyCon "IO"
maybeTc    = mkTyCon "Maybe"
eitherTc   = mkTyCon "Either"
listTc     = mkTyCon "[]"

unitTc, orderingTc, arrayTc, complexTc, handleTc :: TyCon
unitTc     = mkTyCon "()"
orderingTc = mkTyCon "Ordering"
arrayTc    = mkTyCon "Array"
complexTc  = mkTyCon "Complex"
handleTc   = mkTyCon "Handle"

-- Hugs/GHC extension lib types:
addrTc, stablePtrTc, mvarTc :: TyCon
addrTc       = mkTyCon "Addr"
stablePtrTc  = mkTyCon "StablePtr"
mvarTc       = mkTyCon "MVar"

foreignObjTc, stTc :: TyCon
foreignObjTc = mkTyCon "ForeignObj"
stTc         = mkTyCon "ST"

int8Tc, int16Tc, int32Tc, int64Tc :: TyCon
int8Tc       = mkTyCon "Int8"
int16Tc      = mkTyCon "Int16"
int32Tc      = mkTyCon "Int32"
int64Tc	     = mkTyCon "Int64"

word8Tc, word16Tc, word32Tc, word64Tc :: TyCon
word8Tc      = mkTyCon "Word8"
word16Tc     = mkTyCon "Word16"
word32Tc     = mkTyCon "Word32"
word64Tc     = mkTyCon "Word64"

tyConTc, typeRepTc, dynamicTc :: TyCon
tyConTc      = mkTyCon "TyCon"
typeRepTc    = mkTyCon "Type"
dynamicTc    = mkTyCon "Dynamic"

-- GHC specific:
{- BEGIN_FOR_GHC
byteArrayTc, mutablebyteArrayTc, wordTc :: TyCon
byteArrayTc  = mkTyCon "ByteArray"
mutablebyteArrayTc = mkTyCon "MutableByteArray"
wordTc       = mkTyCon "Word"
   END_FOR_GHC -}

\end{code}

begin{code}
test1,test2, test3, test4 :: Dynamic

test1 = toDyn (1::Int)
test2 = toDyn ((+) :: Int -> Int -> Int)
test3 = dynApp test2 test1
test4 = dynApp test3 test1

test5, test6,test7 :: Int
test5 = fromDyn test4 0
test6 = fromDyn test1 0
test7 = fromDyn test2 0

test8 :: Dynamic
test8 = toDyn (mkAppTy listTc)

test9 :: Float
test9 = fromDyn test8 0

printf :: String -> [Dynamic] -> IO ()
printf str args = putStr (decode str args)
 where
  decode [] [] = []
  decode ('%':'n':cs) (d:ds) =
    (\ v -> show v++decode cs ds) (fromDyn  d (0::Int))
  decode ('%':'c':cs) (d:ds) =
    (\ v -> show v++decode cs ds) (fromDyn  d ('\0'))
  decode ('%':'b':cs) (d:ds) =
    (\ v -> show v++decode cs ds) (fromDyn  d (False::Bool))
  decode (x:xs) ds = x:decode xs ds

test10 :: IO ()
test10 = printf "%n = %c, that much is %b\n" [toDyn (3::Int),toDyn 'a', toDyn False]
end{code}


