#!/usr/bin/ruby -Ku
# 
# 	クラスタリング・マッピング最適化
# 	Optimize clustering and mapping to minimize cable/power/energy
# 	Ikki Fujiwara <ikki@nii.ac.jp>
# 

RACK_WIDTH     =  60 # Rack width [cm]
RACK_DEPTH     = 210 # Rack depth including aisle [cm]
OVERHEAD_OUTER = 200 # Inter-rack cabling overhead [cm]
OVERHEAD_INNER = 100 # Intra-rack cabling overhead [cm]
BANDWIDTH      =  40 # Bandwidth [Gbps] (affects cable cost)

$help_message = <<-EOM
Usage: #{$0} [Options] Input_file Clustering Mapping

Options:
  -r                reproduce the results from log file; don't run the solver
  -f                overwrite existing file
  -q                be quiet
  -v<LEVEL>         be verbose
  --traffic=<FILE>  per-node traffic filename to calculate energy consumption
  --euclidean       use diagonal cabling in place of Manhattan cabling

Input_file:
  *.edges           per-node edgelist with cable length

Clustering:
  {seq|metis}       seq: sequential clustering (baseline)
                    metis: optimal clustering by METIS

Mapping:
  {ca|po|en}-{ts|sa|base|oneway|zigzag}

  The former is the objective value to minimize.
                    ca: cable length [cm]
                    po: power consumption [mW]
                    en: energy consumption [pJ] considering traffic

  The latter is the optimization method.
                    ts: taboo search by Taillard
                    sa: simulated annealing by Taillard
                    base: better one of oneway and zigzag (baseline)
                    oneway: trivial, good for meshes
                    zigzag: trivial, good for rings

  Example: 'ca-ts' minimizes cable length by taboo search.

Files created:
  *.*.*.coord       coordinates of nodes and racks on the floor
  *.*.*.edges       per-node edgelist with cable length
EOM

require "optparse"
require "tempfile"
require "./lay.rb"

def main
	$opts = ARGV.getopts("rfqv:", "traffic:", "euclidean", "help")
	$opts["v"] = $opts["v"].to_i
	seed = $opts["seed"].to_i
	if $opts["help"] || ARGV.length < 3
		puts $help_message
		exit(1)
	end
	g = MyNetwork.new(false)
	topology = File.basename(ARGV[0], ".*")
	ext_edges = File.extname(ARGV[0])
	$myname = topology + "." + ARGV[1] + "." + ARGV[2]
	fnames = Hash.new
	fnames["in_coord"] = topology.split(/[.+]/)[0] + ".coord"
	fnames["in_edges"] = ARGV[0]
	fnames["out_coord"] = $myname + ".coord"
	fnames["out_edges"] = $myname + ext_edges
	fnames["log"] = $myname + ".log"
	optim["value"] = ARGV[2].split("-")[0]
	optim["method"] = ARGV[2].split("-")[1]
	if $opts["r"] && $opts["f"]
		STDERR.puts "Options -r and -f cannot be used at the same time."
		exit(1)
	end
	if $opts["r"] && !File.size?(fnames["log"])
		STDERR.puts fnames["log"] + " must exist in the same directory to reproduce the result."
	end
	if $opts["traffic"] && !File.size?($opts["traffic"])
		STDERR.puts $opts["traffic"] + " is not available."
		exit(1)
	end
	if !$opts["traffic"] && optim["value"] == "en"
		STDERR.puts "Traffic file must be given as --traffic=<FILE> to calculate energy consumption."
		exit(1)
	end
	if !File.size?(fnames["in_coord"])
		STDERR.puts fnames["in_coord"] + " must exist in the same directory."
		exit(1)
	end
	if !File.size?(fnames["in_edges"])
		STDERR.puts fnames["in_edges"] + " is not available."
		exit(1)
	end
	g.load_geometry(fnames["in_coord"])
	g.load_edgelist(fnames["in_edges"])
	
	if ARGV[1] == "metis"
		g.optimize_clustering
	end
	
	rack_conn = g.to_rack_matrix
	
	rack_conn2 = MyMatrix.new.load_edgelist(ARGV[3]) # 確認用
	puts rack_conn == rack_conn2
	
	return
	# ここまで書いた
	
	
	#===========================================================
	#   Input
	#===========================================================
	# Prepare per-rack connection matrix and distance matrix
	rack_conn = MyMatrix.new.load_edgelist(ARGV[3])
	raise "Square matrix expected" unless rack_conn.row_size == rack_conn.col_size
	layout = RackLayout.new(rack_conn.row_size).load_coordinates(fnames["in_coord"])
	if $opts["euclidean"]
		dist = layout.l2norm_matrix
	else
		dist = layout.l1norm_matrix
	end
	# Prepare per-node connection matrix
	node_conn = MyMatrix.new
	node_conn.load_edgelist(fnames["in_edges"], true) # directed
	# Prepare traffic matrix
	inner_traf = 0
	rack_traf = MyMatrix.new
	node_traf = MyMatrix.new
	if $opts["traffic"]
		node_traf.load_edgelist($opts["traffic"])
		node_traf.each do |v1, v2, w|
			if g.rack_of[v1] == g.rack_of[v2]
				inner_traf += w
			else
				rack_traf[g.rack_of[v1], g.rack_of[v2]] += w
			end
		end
	end
	# Prepare map
	case optim["value"]
	when "ca"
		q = QAP.new(dist, rack_conn)
	when "po"
		q = QAP.new(dist.map{|val| power_of(cable_of(val)) }, rack_conn)
	when "en"
		q = QAP.new(dist.map{|val| energy_of(cable_of(val)) }, rack_traf)
	else
		STDERR.puts "Unknown objective value: " + optim["value"]
		exit(1)
	end
	# Prepare solver
	case optim["method"]
	when "sa"
		q.solver = SaQapSolver.new(100000000, 10)
	when "ts"
		q.solver = RoTsQapSolver.new
	when "oneway"
		q.solver = BaselineQapSolver.new(layout, "oneway")
	when "zigzag"
		q.solver = BaselineQapSolver.new(layout, "zigzag")
	when "base"
		q.solver = BaselineQapSolver.new(layout, nil)
	else
		STDERR.puts "Unknown optimization method: " + optim["method"]
		exit(1)
	end
	
	#===========================================================
	#   Solve
	#===========================================================
	if $opts["f"] || (!$opts["r"] && !File.size?(fnames["log"]))
		open(fnames["log"], "w") do |logf|
			logf.sync = true
			q.solve(logf)
		end
	else
# 		STDERR.puts "Reproducing the result from '#{fnames['log']}'" unless $opts["q"]
		open(fnames["log"], "r") do |logf|
			q.reproduce(logf)
		end
	end
	
	#===========================================================
	#   Output
	#===========================================================
	# Write per-node edgelist and coordinates
	open(fnames["out_edges"], "w") do |f|
		node_conn.each do |v1, v2, w|
			len = cable_of(dist[q.location[g.rack_of[v1]], q.location[g.rack_of[v2]]])
			f.printf("%d %d %d\n", v1, v2, len)
		end
	end
	open(fnames["out_coord"], "w") do |f|
		g.rack_of.each_with_index do |rack, node|
			x, y = layout.coord(q.location[rack])
			f.printf("%d %d %d %d\n", node, rack, x, y)
		end
	end
end

# 行列
# 	行番号・列番号は0オリジン
class MyMatrix
	include Enumerable
	def initialize
		@data = Hash.new{|hash, key|
			hash[key] = Hash.new(0) # 行データのデフォルト値
		}
		@row_size = 0
		@col_size = 0
	end
	attr_reader :row_size, :col_size
	def initialize_copy(org)
		@data = Hash.new{|hash, key|
			hash[key] = Hash.new(0) # デフォルト値
		}
		org.each do |i, j, val|
			@data[i][j] = val
		end
	end
	def [](i, j)
		@data[i][j]
	end
	def []=(i, j, val)
		raise TypeError unless i.is_a?(Integer) && j.is_a?(Integer) && val.is_a?(Numeric)
		@data[i][j] = val
		@row_size = i + 1 if row_size < i + 1
		@col_size = j + 1 if col_size < j + 1
		self
	end
	# スカラー和
	def +(other)
		raise TypeError unless other.is_a?(Numeric)
		a = self.class.new
		@row_size.times do |i|
			@col_size.times do |j|
				a[i, j] = @data[i][j] + other
			end
		end
		a
	end
	# スカラー積
	def *(other)
		raise TypeError unless other.is_a?(Numeric)
		a = self.class.new
		@data.each do |i, r|
			r.each do |j, val|
				a[i, j] = val * other
			end
		end
		a
	end
	# 各要素にブロックを適用した結果を新しい行列で返す
	def map
		a = self.class.new
		@data.each do |i, r|
			r.each do |j, val|
				a[i, j] = yield(val)
			end
		end
		a
	end
	def empty?
		@data.empty?
	end
	def each
		@data.each do |i, r|
			r.each do |j, val|
				yield(i, j, val)
			end
		end
	end
	def each_index
		@row_size.times do |i|
			@col_size.times do |j|
				yield(i, j)
			end
		end
	end
	def each_index_upper(strict = false)
		raise "Square matrix expected" unless @row_size == @col_size
		d = (strict) ? 1 : 0
		(0...(@row_size - d)).each do |i|
			((i + d)...@col_size).each do |j|
				yield(i, j)
			end
		end
	end
	def along_row(i)
		@data[i].each do |j, val|
			yield(j, val)
		end
	end
	def along_col(j)
		@data.each do |i, r|
			yield(i, r[j])
		end
	end
	def to_s
		a = Array.new
		@row_size.times do |i|
			b = Array.new
			@col_size.times do |j|
				b.push(@data[i][j].to_s)
			end
			a.push(b.join(" "))
		end
		a.join("\n")
	end
	# スペース区切り文字列を読む
	def scan(str)
		str.each_with_index do |line, i|
			line.split.each_with_index do |val, j|
				self[i.to_i, j.to_i] = val.to_i
			end
		end
		self
	end
	# エッジリストを隣接行列として読み込む
	def load_edgelist(filename, directed = false)
		IO.foreach(filename) do |line|
			i, j, val = line.split # split by whitespaces
			next unless i && j
			i = i.to_i
			j = j.to_i
			val = (val) ? val.to_i : 1
			self[i, j] = val
			self[j, i] = val if !directed
		end
		self
	end
end


# 重複グラフ
class MyMultigraph
	def to_matrix(directed = false)
		mat = MyMatrix.new
		each do |i, j, w|
			mat[i, j] = w
			mat[j, i] = w if !directed
		end
		mat
	end
end

# 座標つき単純グラフ
class MyMap
	attr_reader :rack_of
	# 配線長を値とするノード行列
	def to_node_matrix()
		mat = MyMatrix.new
		each do |n1, n2|
			mat[n1, n2] = cable_between(n1, n2)
		end
		mat
	end
	# 配線数を値とするラック行列
	def to_rack_matrix(filename)
		grack = MyMultigraph.new(false)
		each do |n1, n2|
			r1 = @rack_of[n1]
			r2 = @rack_of[n2]
			grack.add_edge(r1, r2, 1) if r1 != r2
		end
		grack.to_matrix
	end
	# クラスタリングを METIS で最適化
	def optimize_clustering()
		@nodes_in = Hash.new # @nodes_in[ラック番号] = ノードリスト
		@rack_of  = Hash.new # @rack_of[ノード番号] = ラック番号
		nnodes = @nodes.length # ノード数
		nparts = (nnodes.to_f / @racksize).ceil # 分割数
		mtsfile = "#{Dir::tmpdir}/pid#{$$}.metis"
		outfile = "#{mtsfile}.part.#{nparts}"
		# Write input file
		open(mtsfile, "w") do |f|
			f.puts "#{vertex_count} #{edge_count}"
			vertices.each do |i|
				# Vertex ID is 1-origin
				a = vertices.select{|j| has_edge?(i, j) }.collect{|x| x + 1 }
				f.puts a.join(" ")
			end
		end
		# Run solver
		maxsize = Float::MAX
		100.times do |t|
			res = `gpmetis -ufactor=1 -seed=#{t} #{mtsfile} #{nparts}`
# 			edgecut = res.scan(/Edgecut: *(\d+)/)[0][0].to_i
# 			commvol = res.scan(/communication volume: *(\d+)/)[0][0].to_i
			maxsize = res.scan(/actual: *(\d+)/)[0][0].to_i
			break if maxsize <= @racksize
		end
		if @racksize < maxsize
			STDERR.puts "Warning: #{$myname}: Enlarged rack size from #{@racksize} to #{maxsize}" if !$opts["q"]
			@racksize = maxsize # ラックサイズを変更!
		end
		# Read output file
		open(outfile, "r") do |f|
			f.each_line.with_index do |r, n|
				n = n.to_i # ノード番号
				r = r.to_i # ラック番号
				@nodes_in[r] ||= Array.new
				@nodes_in[r].push(n)
				@rack_of[n] = r
			end
		end
		File.delete(mtsfile, outfile)
	end
end

# ラック配置
class RackLayout
	def initialize(n)
		@nrack = n.to_i # ラック数
		@width = RACK_WIDTH # ラック幅 (cm)
		@depth = RACK_DEPTH # ラック奥行＋通路幅 (cm)
		@tile = @width.gcd(@depth) # 最大公約数
		if @nrack > 0
			@ny = 1 # 奥行方向の設置数
			@nx = @nrack # 幅方向の設置数
		else
			@ny = 0
			@nx = 0
		end
	end
	attr_reader :nrack, :nx, :ny, :width, :depth, :aisle, :tile  # , :rack_of
	# 部屋の幅
	def room_width
		@nx * @width
	end
	# 部屋の奥行
	def room_depth
		@ny * @depth
	end
	# 部屋の面積
	def room_area
		room_width * room_depth
	end
	# 最長距離
	def max_length
		room_width + room_depth
	end
	# 列ごとにイテレート
	def each_row
		(0...@nrack).each_slice(@nx) do |row|
			yield(row)
		end
	end
	def each_row_with_index
		i = 0
		(0...@nrack).each_slice(@nx) do |row|
			yield(row, i)
			i += 1
		end
	end
	# ラックの座標 [x, y]
	def coord(i)
		[(i % @nx) * @width, (i / @nx) * @depth]
	end
	# マンハッタン距離
	def l1norm(i, j)
		x = (i.to_i % @nx - j.to_i % @nx) * @width
		y = (i.to_i / @nx - j.to_i / @nx) * @depth
		x.abs + y.abs
	end
	# ユークリッド距離
	def l2norm(i, j)
		x = (i.to_i % @nx - j.to_i % @nx) * @width
		y = (i.to_i / @nx - j.to_i / @nx) * @depth
		Math.sqrt(x * x + y * y).round
	end
	# マンハッタン距離行列
	def l1norm_matrix
		a = MyMatrix.new
		@nrack.times do |i|
			@nrack.times do |j|
				a[i, j] = a[j, i] = l1norm(i, j)
			end
		end
		a
	end
	# ユークリッド距離行列
	def l2norm_matrix
		a = MyMatrix.new
		@nrack.times do |i|
			@nrack.times do |j|
				a[i, j] = a[j, i] = l2norm(i, j)
			end
		end
		a
	end
	# 座標ファイルを読み込む
	def load_coordinates(filename)
		rmax = 0
		xmax = 0
		ymax = 0
		IO.foreach(filename) do |line|
			n, r, x, y = line.split # split by whitespaces
			next unless n && r && x && y
			n = n.to_i
			r = r.to_i
			x = x.to_i
			y = y.to_i
			raise "Rack width unmatch with coordinates" unless x % @width == 0
			raise "Rack depth unmatch with coordinates" unless y % @depth == 0
			rmax = r if rmax < r
			xmax = x if xmax < x
			ymax = y if ymax < y
# 			@rack_of[n] = r
		end
		raise "Number of racks unmatch in edgelist and in coordinates" unless rmax + 1 == @nrack
		@nx = xmax / @width + 1
		@ny = ymax / @depth + 1
		self
	end
end

# 二次割当問題
class QAP
	def initialize(dist_matrix, flow_matrix)
		@dist = dist_matrix # 距離行列
		@flow = flow_matrix # 重み行列
		raise "Distance matrix is not square (#{@dist.row_size} rows, #{@dist.col_size} cols)" unless @dist.row_size == @dist.col_size
		raise "Flow matrix is not square (#{@flow.row_size} rows, #{@flow.col_size} cols)" unless @flow.row_size == @flow.col_size
		raise "Unmatch matrix size (dist: #{@dist.row_size}x#{@dist.col_size}, flow: #{@flow.row_size}x#{@flow.col_size})" unless @dist.row_size == @flow.row_size
		@size = @dist.row_size # 問題サイズ
		@solver = nil # ソルバー
		@solution = Array.new(@size) # 解 @solution[位置] = 施設
		@location = Array.new(@size) # 解 @location[施設] = 位置
		@objective = nil # 目的関数値
		@iterations = nil # 反復回数
		@inverse = false # 距離行列と重み行列を入れ替えて評価するフラグ
		@elapsed = 0 # 計算時間
	end
	attr_reader :size, :dist, :flow
	attr_accessor :solver, :solution, :location, :objective, :iterations, :inverse, :elapsed
	# 求解
	def solve(logf)
		raise "No solver set" unless @solver.is_a?(QapSolver)
		@solver.run(self, logf)
	end
	# ログファイルから再現
	def reproduce(logf)
		raise "No solver set" unless @solver.is_a?(QapSolver)
		@solver.reproduce(self, logf)
	end
	# QAPLIB形式の文字列
	def to_s
		"#{@size}\n\n#{@dist}\n\n#{@flow}\n"
	end
	# 評価関数値を計算
	def evaluate(sol)
		raise "Unmatch size (matrix=#{@size}x#{@size}, solution=#{sol.size})" unless @size == sol.size
		raise "Cannot invert the solution. Maybe 1-origin?" unless sol.sort.uniq == (0...@size).to_a
		sum = 0
		unless @inverse
			@dist.each do |i, j, w|
				sum += w * @flow[sol[i], sol[j]]
			end
		else
			@flow.each do |i, j, w|
				sum += w * @dist[sol[i], sol[j]]
			end
		end
		sum / 2
	end
	# 評価関数値を重みなしで計算
	def evaluate_unweighted(sol)
		raise "Unmatch size (matrix=#{@size}x#{@size}, solution=#{sol.size})" unless @size == sol.size
		raise "Cannot invert the solution. Maybe 1-origin?" unless sol.sort.uniq == (0...@size).to_a
		sum = 0
		unless @inverse
			@dist.each do |i, j, w|
				sum += w if @flow[sol[i], sol[j]] > 0
			end
		else
			@flow.each do |i, j, w|
 				sum += w if @dist[sol[i], sol[j]] > 0
			end
		end
		sum / 2
	end
end

# 二次割当ソルバー
class QapSolver
	def run(q, logf)
		start_time = Time.now
		logf.puts start_time.strftime("===== %Y-%m-%d %H:%M:%S =====")
		if q.size > 0
			logf.puts(@cmd)
			IO.popen(@cmd, "r+") do |pipe|
				write_input(q, pipe)
				pipe.close_write
				read_output(q, pipe, logf)
			end
			q.solution.each_with_index do |x, i|
				q.location[x] = i
			end
		else
			q.iterations = 0
			q.objective = 0
			q.solution = []
			q.location = []
		end
		finish_time = Time.now
		logf.puts finish_time.strftime("===== %Y-%m-%d %H:%M:%S =====")
		q.elapsed = finish_time - start_time
	end
	def reproduce(q, logf)
		read_output(q, logf, nil)
		q.solution.each_with_index do |x, i|
			q.location[x] = i
		end
		logf.rewind
		ts = logf.read.scan(/^===== (\d+-\d+-\d+ \d+:\d+:\d+) =====$/)
		start_time = Time.parse(ts[0][0])
		finish_time = Time.parse(ts[-1][0])
		q.elapsed = finish_time - start_time
	end
end

class TaillardQapSolver < QapSolver
	def write_input(q, f)
		f.puts(q)
	end
	def read_output(q, f, logf)
		f.tee(logf).scan(/^@(\d+)\s+(\d+)\s+\[([\d,\s]+)\]/).each do |m|
			if !q.iterations || q.iterations < m[0].to_i
				# 反復回数の最大値
				q.iterations = m[0].to_i
			end
			if !q.objective || q.objective > m[1].to_i / 2
				# 目的関数の最小値と、そのときの解
				q.objective = m[1].to_i / 2
				q.solution = m[2].scan(/\d+/).collect{|n| n.to_i }
			end
		end
	end
end

# Simulated Annealing
class SaQapSolver < TaillardQapSolver
	def initialize(n_iteration, n_trial)
		here = File.dirname(File.expand_path(__FILE__))
		@cmd = "#{here}/sa_qap #{n_iteration} #{n_trial}"
	end
end

# Robust Taboo Search
class RoTsQapSolver < TaillardQapSolver
	def initialize
		here = File.dirname(File.expand_path(__FILE__))
		@cmd = "#{here}/tabou_qap"
	end
end

# Baseline
class BaselineQapSolver < QapSolver
	def initialize(layout, method = nil)
		raise "No layout given" unless layout.is_a?(RackLayout)
		@lay = layout
		@method = method
	end
	def run(q, logf)
		if q.size > 0
			case @method
			when "oneway"
				q.solution = sol_oneway
			when "zigzag"
				q.solution = sol_zigzag
			else
				sols = [sol_oneway, sol_zigzag]
				q.solution = sols.min_by{|sol| q.evaluate(sol) }
			end
			q.iterations = 0
			q.objective = q.evaluate(q.solution)
			q.solution.each_with_index do |x, i|
				q.location[x] = i
			end
		else
			q.iterations = 0
			q.objective = 0
			q.solution = []
			q.location = []
		end
	end
	def sol_oneway
		sol = Array.new(@lay.nrack)
		@lay.each_row_with_index do |row, i|
			sol[i * @lay.nx, row.length] = row
		end
		sol
	end
	def sol_zigzag
		sol = Array.new(@lay.nrack)
		@lay.each_row_with_index do |row, i|
			row = row.reverse if i.odd?
			sol[i * @lay.nx, row.length] = row
		end
		sol
	end
end

class IO
	# ログファイルに書き出しつつ文字列として返す
	def tee(f)
		str = ""
		self.each_line do |line|
			line = line.chomp
			f.puts(line) if f
			str << line << "\n"
		end
		str
	end
end

main if $0 == __FILE__
