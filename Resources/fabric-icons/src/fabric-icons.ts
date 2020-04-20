  // Your use of the content in the files referenced here is subject to the terms of the license at https://aka.ms/fabric-assets-license

// tslint:disable:max-line-length

import {
  IIconOptions,
  IIconSubset,
  registerIcons
} from '@uifabric/styling';

export function initializeIcons(
  baseUrl: string = '',
  options?: IIconOptions
): void {
  const subset: IIconSubset = {
    style: {
      MozOsxFontSmoothing: 'grayscale',
      WebkitFontSmoothing: 'antialiased',
      fontStyle: 'normal',
      fontWeight: 'normal',
      speak: 'none'
    },
    fontFace: {
      fontFamily: `"FabricMDL2Icons"`,
      src: `url('${baseUrl}fabric-icons-6c64f64d.woff') format('woff')`
    },
    icons: {
      'Settings': '\uE713',
      'Attach': '\uE723',
      'Accept': '\uE8FB',
      'Repair': '\uE90F',
      'EmojiDisappointed': '\uEA88',
      'StatusErrorFull': '\uEB90',
      'AddTo': '\uECC8',
      'ExploreContent': '\uECCD',
      'CirclePauseSolid': '\uF2D8',
      'CirclePause': '\uF2D9',
      'MSNVideosSolid': '\uF2DA',
      'CircleStopSolid': '\uF2DB'
    }
  };

  registerIcons(subset, options);
}
