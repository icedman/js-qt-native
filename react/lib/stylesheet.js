import React from 'react';

const styleMap = {
    backgroundColor: 'background',
    fontSize: 'font-size'
}

const styleExclude = [
]

const distillStyle = (style) => {
    let res = {}
    Object.keys(style).forEach(k => {
        switch(k) {
            case 'marginHorizontal':
                res['margin-left'] = style[k];
                res['margin-right'] = style[k];
            return;
            case 'marginVertical':
                res['margin-top'] = style[k];
                res['margin-bottom'] = style[k];
            return;
        }
        if (styleMap[k]) {
            res[styleMap[k]] = style[k];
            return;
        }

        if (styleExclude.indexOf(k) != -1) {
            return;
        }
        res[k] = style[k];
    })
    return res;
}

const StyleSheet = {
    create: (styles) => {
        let res = {};
        Object.keys(styles).forEach(k => {
            res[k] = distillStyle(styles[k]);
        })
        return res;
    }
}

export default StyleSheet