module Stable where 

data StableName a -- abstract

primitive makeStableName   :: a -> IO (StableName a)
primitive deRefStableName  :: StableName a -> a
primitive hashStableName   :: StableName a -> Int
primitive eqStableName	   :: StableName a -> StableName a -> Int

instance Eq (StableName a) where
    (==) = eqStableName
