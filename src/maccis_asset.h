enum asset_types
{
  ASSET_BITMAP, ASSET_RAW_MODEL, ASSET_STRUCT_ARRAY, ASSET_FONT
};

struct asset_wrapper
{
  asset_wrapper *pNext;
  void *asset;
  unsigned int assetType;
  unsigned int assetSize; //size of asset in bytes!
};

struct loaded_asset
{
  unsigned int count; //amount of assets
  asset_wrapper *pWrapper;
};
