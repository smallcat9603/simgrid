#!/usr/bin/ruby -Ku
# 
# 	グラフ分割 (2分割帯域幅を求める)
# 	Graph partitioning (to calculate bisection bandwidth)
# 	Ikki Fujiwara <ikki@nii.ac.jp>
# 

$help_message = <<-EOM
Usage: #{$0} [Options]... Input_file [Num_parts]

Options:
  -v            be verbose
  -c            create clustered (per-rack) edgelist file

Input_file:     *.edges
Num_parts:      number of partitions/clusters [default: 2]
EOM

require "optparse"
require "pp"

def main
	$opts = ARGV.getopts("cv", "help")
	if $opts["help"] || ARGV.length < 1
		puts $help_message
		exit(1)
	end
	myname = File.basename(ARGV[0], ".*")
	
	g = MyGraphPartition.new(ARGV[0], ARGV[1])
	g.solve
	g.save_clustered_edgelist if $opts["c"]
	
	STDOUT.puts format("%s\t%d", myname, g.edgecut)
	STDOUT.puts g.report if $opts["v"]
end

# 配列
class Array
	# 要素を適当に数値化
	def to_nums
		self.collect do |x|
			if x.class == String
				x.to_num
			else
				(x.to_i == x.to_f) ? x.to_i : x.to_f
			end
		end
	end
	# 和
	def sum
		self.inject(0){|x, y| x + y }
	end
	# 積
	def mul
		self.inject(1){|x, y| x * y }
	end
	# 平均
	def avg
		self.sum.to_f / self.length
	end
	# ヒストグラム
	def histogram
		h = Hash.new(0)
		self.sort.each do |x|
			h[x] += 1
		end
		h
	end
	# 複数の配列の同じ位置にある要素同士を組み合わせてイテレート
	# 	[[1,2,3,4],[5,6,7],[8,9]].each_pair => [1,5,8],[2,6,9],[3,7,nil],[4,nil,nil]
	def each_pair
		self.max_by{|x| x.length }.each_index do |i|
			yield(self.collect{|x| x[i] })
		end
	end
	# 複数の配列からそれぞれ1個の要素を選んだ配列の配列を返す
	# 	[[1,2],[3,4,5]].self_product => [[1,3],[1,4],[1,5],[2,3],[2,4],[2,5]]
	def self_product
		if self.empty?
			[[]]
		elsif self[0].is_a?(Array)
			self[0].product(*self[1..-1])
		else
			raise TypeError
		end
	end
	# ランダムマッチングを生成
	# 	ブロックにペア案が渡される。ブロックの値が真だったペアをすべて含む配列を返す
	def random_matching
		pairs = Array.new
		pool = self.shuffle
		until pool.empty?
			x = pool.shift
			pool.each_with_index do |y, i|
				if yield(x, y)
					pool.delete_at(i)
					pairs.push([x, y])
					break
				end
			end
		end
		pairs
	end
end

# 重複グラフ
# 	辺の本数を辺の width で表現する。width == 0 の辺は存在しない。次数は width の合計。
class MyMultigraph
	include Enumerable
	attr_reader :edge_data, :vertex_data, :edge_count
	def initialize(directed)
		@directed = directed
		@edge_data = Hash.new{|hash, key| hash[key] = Hash.new(0) }
		@edge_count = 0
		@vertex_data = Hash.new{|hash, key| hash[key] = Hash.new(0) }
	end
	def each
		@edge_data.each do |i, h|
			h.each do |j, w|
				yield(i, j, w)
			end
		end
	end
	def edges
		sort
	end
	def vertices
		@vertex_data.keys.sort
	end
	def vertex_count
		@vertex_data.length
	end
	def include_edge?(i, j)
		@edge_data.has_key?(i) && @edge_data[i].has_key?(j)
	end
	private :include_edge?
	def has_edge?(i, j)
		i, j = j, i if !(@directed || i <= j)
		include_edge?(i, j)
	end
	def has_vertex?(k)
		@vertex_data.has_key?(k)
	end
	def width(i, j)
		i, j = j, i if !(@directed || i <= j)
		(include_edge?(i, j)) ? @edge_data[i][j] : nil
	end
	def degree(k)
		(has_vertex?(k)) ? @vertex_data[k][:outdegree] + @vertex_data[k][:indegree] : nil
	end
	def outdegree(k)
		(has_vertex?(k)) ? @vertex_data[k][:outdegree] : nil
	end
	def indegree(k)
		(has_vertex?(k)) ? @vertex_data[k][:indegree] : nil
	end
	def degrees
		a = Array.new
		@vertex_data.each_key do |k|
			a[k] = degree(k)
		end
		a
	end
	def add_edge?(i, j, w = 1)
		# w = 0: allowed but does nothing
		raise ArgumentError "Negative width is not allowed" unless w >= 0
		i, j = j, i if !(@directed || i <= j)
		if w > 0
			@edge_data[i][j] += w
			@edge_count += w
			@vertex_data[i][:outdegree] += w
			@vertex_data[j][:indegree] += w
			self
		else
			nil
		end
	end
	def add_edge(i, j, w = 1)
		add_edge?(i, j, w)
		self
	end
	def delete_edge?(i, j, w = 0)
		# w = 0: delete the whole edge
		raise ArgumentError "Negative width is not allowed" unless w >= 0
		i, j = j, i if !(@directed || i <= j)
		if include_edge?(i, j)
			if w > 0 && w < @edge_data[i][j]
				@edge_data[i][j] -= w
			else
				w = @edge_data[i][j]
				@edge_data[i].delete(j)
				@edge_data.delete(i) if @edge_data[i].empty?
			end
			@edge_count -= w
			@vertex_data[i][:outdegree] -= w
			@vertex_data[j][:indegree] -= w
			self
		else
			nil
		end
	end
	def delete_edge(i, j, w = 0)
		delete_edge?(i, j, w)
		self
	end
	def scan_edge(str)
		if s = str.match(/^\s*(\d+)\s+(\d+)\s+(\d+)/)
			add_edge?(s[1].to_i, s[2].to_i, s[3].to_i)
		else
			raise "Edge string must begin with three numbers separated by whitespaces: '#{str}'"
		end
	end
	def format_edge(i, j, w)
		format("%d %d %d", i, j, w)
	end
	def load_edgelist(filename)
		open(filename, "r") do |f|
			f.each_line do |line|
				scan_edge(line)
			end
		end
	end
	def save_edgelist(filename)
		open(filename, "w") do |f|
			edges.each do |i, j, w|
				f.puts(format_edge(i, j, w))
			end
		end
	end
end

# 単純グラフ
class MyGraph < MyMultigraph
	def add_edge?(i, j, w = nil)
		if i != j && !has_edge?(i, j)
			super(i, j)
		else
			nil
		end
	end
	def scan_edge(str)
		if s = str.match(/^\s*(\d+)\s+(\d+)/)
			add_edge?(s[1].to_i, s[2].to_i)
		else
			raise "Edge string must begin with two numbers separated by whitespaces: '#{str}'"
		end
	end
	def format_edge(i, j, w = nil)
		format("%d %d", i, j)
	end
end

# グラフ分割問題
class MyGraphPartition < MyGraph
	def initialize(filename, nparts)
		super(false)
		@filename = filename
		@nparts = (nparts) ? nparts.to_i : 2
		@edgecut = 0
		@commvol = 0
		@racksize = 0
		@rack_of = Array.new
		load_edgelist(filename)
	end
	attr_reader :report, :edgecut, :commvol
	# 求解
	def solve
		raise "Num_parts must be > 1" unless @nparts > 1
		mtsfile = @filename.sub(/\.\w+$/){ ".metis" }
		outfile = "#{mtsfile}.part.#{@nparts}"
		# Write input file
		open(mtsfile, "w") do |f|
			f.puts "#{vertex_count} #{edge_count}"
			vertices.each do |i|
				a = vertices.select{|j| has_edge?(i, j) }
				a = a.collect{|x| x + 1 } # Vertex ID is 1-origin
				f.puts a.join(" ")
			end
		end
		# Run solver
		@report = `gpmetis -ufactor=1 #{mtsfile} #{@nparts}`
		@edgecut = @report.scan(/Edgecut: *([\d.]+)/)[0][0]
		@commvol = @report.scan(/communication volume: *([\d.]+)/)[0][0]
		@racksize = @report.scan(/actual: *([\d.]+)/)[0][0]
		# Read output file
		open(outfile, "r") do |f|
			f.each_line.with_index do |rack, node|
				@rack_of[node] = rack.to_i
			end
		end
		File.delete(mtsfile, outfile)
	end
	# 配線数を重みとするラック単位のエッジリストを書き出す
	def save_clustered_edgelist
		clufile = @filename.sub(/(\.edges)?$/){ ".metis-#{@racksize}.edges" }
		logfile = @filename.sub(/(\.edges)?$/){ ".metis-#{@racksize}.log" }
		grack = MyMultigraph.new(false)
		each do |n1, n2|
			r1 = @rack_of[n1]
			r2 = @rack_of[n2]
			grack.add_edge(r1, r2, 1) if r1 != r2
		end
		grack.save_edgelist(clufile)
# 		return
		# ログファイルも作る
		ntube = 0
		nlink = 0
		grack.each do |i, j, w|
			raise "oops" unless i < j
			ntube += 1
			nlink += w
		end
		open(logfile, "w") do |f|
			f.printf("#node      = %d\n", vertex_count)
			f.printf("#edge      = %d\n", edge_count)
			f.printf("#rack      = %d\n", @nparts)
			f.printf("#tube      = %d\n", ntube)
			f.printf("#link      = %d\n", nlink)
			f.printf("membership = %s\n", vertices.collect{|n| @rack_of[n] }.join(","))
		end
	end
end

main if $0 == __FILE__
