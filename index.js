const chartconverter = require('bindings')('chartconverterjs.node')
exports.ConvertChart = chartconverter.ConvertChart;

// const fs = require('fs')

// const smPath = "/mnt/c/dev/stepmania/Songs/Shpadoinkle #9/Acid Wolfpack/Acid Wolfpack.sm"
// const smData = fs.readFileSync(smPath, 'utf8')
// const jsonData = chartconverter.ConvertChart('sm', 'json', smData)
// console.log(jsonData)