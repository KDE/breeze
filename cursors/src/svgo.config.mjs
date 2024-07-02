export default {
    multipass: true,
    js2svg: {
      indent: 4,
      pretty: true,
    },
    plugins: [
        {
            name: 'preset-default',
            params: {
              overrides: {
                cleanupIds: false,
                removeViewBox: false,
                removeHiddenElems: false,
                
                removeUselessDefs: true,
                convertTransform: true,
      
                // customize the params of a default plugin
                inlineStyles: {
                  onlyMatchedOnce: false,
                },
              },
            },
        },
    ],
};